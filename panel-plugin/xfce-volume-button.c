/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2012 Guido Berhoerster <guido+xfce@berhoerster.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gdk/gdkkeysyms.h>

#include <libxfce4ui/libxfce4ui.h>

#include <libxfce4panel/libxfce4panel.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-volume-button.h"



#define VOLUME_EPSILON 0.005



/* Properties */
enum
{
  PROP_0,
  PROP_TRACK_LABEL,
  PROP_IS_CONFIGURED,
  PROP_IS_MUTED,
  N_PROPERTIES,
};



/* Signal identifiers */
enum
{
  VOLUME_CHANGED,
  LAST_SIGNAL,
};



/* Signals */
static guint button_signals[LAST_SIGNAL];



/* Icons for different volume levels */
static const char *icons[] = {
  "audio-volume-muted",
  "audio-volume-low",
  "audio-volume-medium",
  "audio-volume-high",
  NULL
};



static void       xfce_volume_button_class_init     (XfceVolumeButtonClass *klass);
static void       xfce_volume_button_init           (XfceVolumeButton      *button);
static void       xfce_volume_button_dispose        (GObject               *object);
static void       xfce_volume_button_finalize       (GObject               *object);
static void       xfce_volume_button_set_property   (GObject               *object,
                                                     guint                  prop_id,
                                                     const GValue          *value,
                                                     GParamSpec            *pspec);
static void       xfce_volume_button_get_property   (GObject               *object,
                                                     guint                  prop_id,
                                                     GValue                *value,
                                                     GParamSpec            *pspec);
#if 0
static gboolean   xfce_volume_button_key_pressed    (GtkWidget             *widget,
                                                     GdkEventKey           *event,
                                                     XfceVolumeButton      *button);
#endif
static gboolean   xfce_volume_button_button_pressed (GtkWidget             *widget,
                                                     GdkEventButton        *event,
                                                     XfceVolumeButton      *button);
static gboolean   xfce_volume_button_scrolled       (GtkWidget             *widget,
                                                     GdkEventScroll        *event,
                                                     XfceVolumeButton      *button);
static void       xfce_volume_button_volume_changed (XfceVolumeButton      *button,
                                                     gdouble                volume);
static void       xfce_volume_button_update_icons   (XfceVolumeButton      *button,
                                                     GtkIconTheme          *icon_theme);



struct _XfceVolumeButtonClass
{
  GtkButtonClass __parent__;

  /* Signals */
  void (*volume_changed) (XfceVolumeButton *button,
                          gdouble           volume);
};

struct _XfceVolumeButton
{
  GtkButton __parent__;

  /* Image widget for the volume icon */
  GtkWidget  *image;

  /* Adjustment for the volume range and current value */
  GtkObject  *adjustment;

  /* Icon size currently used */
  gint        icon_size;

  /* Array of preloaded icons */
  GdkPixbuf **pixbufs;

  /* Track label used in tooltip */
  gchar      *track_label;

  /* Whether the button is configured */
  gboolean    is_configured;

  /* Mute state of the button */
  gboolean    is_muted;
};



static GObjectClass *xfce_volume_button_parent_class = NULL;



GType
xfce_volume_button_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceVolumeButtonClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_volume_button_class_init,
          NULL,
          NULL,
          sizeof (XfceVolumeButton),
          0,
          (GInstanceInitFunc) xfce_volume_button_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_BUTTON, "XfceVolumeButton", &info, 0);
    }
  
  return type;
}



static void
xfce_volume_button_class_init (XfceVolumeButtonClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_volume_button_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_volume_button_dispose;
  gobject_class->finalize = xfce_volume_button_finalize;
  gobject_class->set_property = xfce_volume_button_set_property;
  gobject_class->get_property = xfce_volume_button_get_property;

  klass->volume_changed = xfce_volume_button_volume_changed;

  g_object_class_install_property (gobject_class,
                                   PROP_TRACK_LABEL,
                                   g_param_spec_string ("track-label",
                                                        "track-label",
                                                        "track-label",
                                                        "Unknown",
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_IS_CONFIGURED,
                                   g_param_spec_boolean ("is-configured",
                                                         "is-configured",
                                                         "is-configured",
                                                         FALSE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_IS_MUTED,
                                   g_param_spec_boolean ("is-muted",
                                                         "is-muted",
                                                         "is-muted",
                                                         TRUE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  button_signals[VOLUME_CHANGED] = g_signal_new ("volume-changed",
                                                 G_TYPE_FROM_CLASS (klass),
                                                 G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                                 G_STRUCT_OFFSET (XfceVolumeButtonClass, volume_changed),
                                                 NULL, 
                                                 NULL,
                                                 g_cclosure_marshal_VOID__DOUBLE,
                                                 G_TYPE_NONE, 
                                                 1, 
                                                 G_TYPE_DOUBLE);
}



static void
xfce_volume_button_init (XfceVolumeButton *button)
{
  button->track_label = NULL;

  button->is_configured = FALSE;

  /* Allocate array for preloaded icons */
  button->pixbufs = g_new0 (GdkPixbuf*, G_N_ELEMENTS (icons)-1);

  /* Create adjustment for the button (from 0.0 to 1.0 in 5% steps) */
  button->adjustment = gtk_adjustment_new (0.0, 0.0, 1.0, 0.05, 0.05, 0.0);

  /* Set to muted by default since the initial adjustment value is 0 */
  button->is_muted = TRUE;

  /* Create a new scaled image for the button icon */
  button->image = xfce_panel_image_new ();
  gtk_container_add (GTK_CONTAINER (button), button->image);
  gtk_widget_show (button->image);

  /* Make the button look flat and make it never grab the focus */
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  gtk_widget_set_can_default (GTK_WIDGET (button), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (button), FALSE);

  /* Connect to button signals */
#if 0
  /* UNUSED FOR NOW DUE TO TOO MUCH PROBLEMS WITH KEYBOARD FOCUS GRABBING */
  g_signal_connect (G_OBJECT (button), "key-press-event", G_CALLBACK (xfce_volume_button_key_pressed), button);
#endif
  g_signal_connect (G_OBJECT (button), "button-press-event", G_CALLBACK (xfce_volume_button_button_pressed), button);
  g_signal_connect (G_OBJECT (button), "scroll-event", G_CALLBACK (xfce_volume_button_scrolled), button);
  g_signal_connect_swapped (gtk_icon_theme_get_default (), "changed", G_CALLBACK (xfce_volume_button_update_icons), button);

  /* Update the state of the button */
  xfce_volume_button_update (button);
}



static void
xfce_volume_button_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_volume_button_parent_class)->dispose) (object);
}



static void
xfce_volume_button_finalize (GObject *object)
{
  guint i;

  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (object);

  /* Free pre-allocated icon pixbufs */
  for (i = 0; i < G_N_ELEMENTS (icons)-1; ++i)
    if (GDK_IS_PIXBUF (button->pixbufs[i]))
      g_object_unref (G_OBJECT (button->pixbufs[i]));
  g_free (button->pixbufs);

  if (button->track_label != NULL)
    {
      g_free (button->track_label);
      button->track_label = NULL;
    }

  (*G_OBJECT_CLASS (xfce_volume_button_parent_class)->finalize) (object);
}



static void xfce_volume_button_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (object);
  gboolean          is_configured;
  gboolean          is_muted;

  switch (prop_id)
    {
      case PROP_TRACK_LABEL:
        g_free (button->track_label);
        button->track_label = g_value_dup_string (value);
        if (button->is_configured)
          xfce_volume_button_update (button);
        break;
      case PROP_IS_MUTED:
        is_muted = g_value_get_boolean (value);
        if (button->is_configured && button->is_muted != is_muted)
          {
            button->is_muted = is_muted;
            xfce_volume_button_update (button);
          }
        break;
      case PROP_IS_CONFIGURED:
        is_configured = g_value_get_boolean (value);
        if (button->is_configured != is_configured)
          {
            button->is_configured = is_configured;
            xfce_volume_button_update (button);
          }
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}



static void xfce_volume_button_get_property (GObject      *object,
                                             guint         prop_id,
                                             GValue       *value,
                                             GParamSpec   *pspec)
{
  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (object);

  switch (prop_id)
    {
      case PROP_TRACK_LABEL:
        g_value_set_string (value, button->track_label);
        break;
      case PROP_IS_MUTED:
        g_value_set_boolean (value, button->is_muted);
        break;
      case PROP_IS_CONFIGURED:
        g_value_set_boolean (value, button->is_configured);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}



GtkWidget*
xfce_volume_button_new (void)
{
  return g_object_new (TYPE_XFCE_VOLUME_BUTTON, NULL);
}



#if 0
static gboolean 
xfce_volume_button_key_pressed (GtkWidget        *widget,
                                GdkEventKey      *event,
                                XfceVolumeButton *button)
{
  gboolean handled = FALSE;
  gdouble  value;
  gdouble  step_increment;
  gdouble  page_size;
  gdouble  min_value;
  gdouble  max_value;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  g_object_get (G_OBJECT (button->adjustment), 
                "value", &value, 
                "step-increment", &step_increment, 
                "page-size", &page_size, 
                "lower", &min_value,
                "upper", &max_value, NULL);

  switch (event->keyval)
    {
      case GDK_plus:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value + step_increment);
        handled = TRUE;
        break;
      case GDK_minus:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value - step_increment);
        handled = TRUE;
        break;
      case GDK_Page_Up:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value + page_size);
        handled = TRUE;
        break;
      case GDK_Page_Down:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value - page_size);
        handled = TRUE;
        break;
      case GDK_Home:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), max_value);
        handled = TRUE;
        break;
      case GDK_End:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), min_value);
        handled = TRUE;
        break;
    }

  xfce_volume_button_update (button);

  g_signal_emit_by_name (button, "volume-changed", gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment)));

  return handled;
}
#endif



static gboolean 
xfce_volume_button_button_pressed (GtkWidget        *widget,
                                   GdkEventButton   *event,
                                   XfceVolumeButton *button)
{
  gboolean mute;

  g_return_val_if_fail (IS_XFCE_VOLUME_BUTTON (button), FALSE);

  /* Check if the middle mouse button was pressed */
  if (event->button == 2)
    {
      /* Only toggle mute if button is in configured state */
      if (button->is_configured)
        {
          /* Determine the new mute state by negating the current state */
          mute = !button->is_muted;

          /* Toggle the button's mute state */
          xfce_volume_button_set_muted (button, mute);
        }

      /* Middle mouse button was handled, do not propagate the event any further */
      return TRUE;
    }

  /* Left and right mouse buttons are ignored, someone else handle it */
  return FALSE;
}



static gboolean 
xfce_volume_button_scrolled (GtkWidget        *widget,
                             GdkEventScroll   *event,
                             XfceVolumeButton *button)
{
  gdouble old_value;
  gdouble new_value;
  gdouble step_increment;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  g_return_val_if_fail (IS_XFCE_VOLUME_BUTTON (button), FALSE);

  /* Ignore scroll events if the button is not in configured state */
  if (!button->is_configured)
    return TRUE;

  /* Get current adjustment value and the step increment size */
  g_object_get (G_OBJECT (button->adjustment), "value", &old_value, "step-increment", &step_increment, NULL);

  /* Distinguish between scroll directions */
  switch (event->direction)
    {
      case GDK_SCROLL_UP:
      case GDK_SCROLL_RIGHT:
        /* Increase one step when scrolling up/right */
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), old_value + step_increment);
        break;
      case GDK_SCROLL_DOWN:
      case GDK_SCROLL_LEFT:
        /* Decrease one step when scrolling down/left */
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), old_value - step_increment);
        break;
    }

  new_value = gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment));
  if (fabs (new_value - old_value) < VOLUME_EPSILON)
    {
      /* Mute when volume reaches 0%, unmute if volume is raised from 0% */
      if (new_value < VOLUME_EPSILON && !button->is_muted)
        xfce_volume_button_set_muted (button, TRUE);
      else if (old_value < VOLUME_EPSILON && button->is_muted)
        xfce_volume_button_set_muted (button, FALSE);
      else
        {
          /* Update the state of the button */
          xfce_volume_button_update (button);
        }

      /* Notify listeners of the new volume */
      g_signal_emit_by_name (button, "volume-changed", new_value);
    }

  /* The scroll event has been handled, stop other handlers from being invoked */
  return TRUE;
}



void 
xfce_volume_button_update (XfceVolumeButton *button)
{
  GdkPixbuf *pixbuf = NULL;
  gdouble    upper;
  gdouble    lower;
  gdouble    value;
  gdouble    range;
  guint      i;
  gchar     *tip_text;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  /* Get upper, lower bounds and current value of the adjustment */
  g_object_get (G_OBJECT (button->adjustment), "upper", &upper, "lower", &lower, "value", &value, NULL);

  /* Determine the difference between upper and lower bound (= volume range) */
  range = (upper - lower) / (G_N_ELEMENTS (icons) - 2);

  if (G_UNLIKELY (!button->is_configured || button->is_muted || value < VOLUME_EPSILON))
    {
      /* By definition, use the first icon if the button is muted or the volume is 0% */
      pixbuf = button->pixbufs[0];
    }
  else
    {
      /* Find the correct icon for the current volume */
      for (i = 1; i < G_N_ELEMENTS (icons) - 1; ++i)
        if (value <= range * i)
          {
            pixbuf = button->pixbufs[i];
            break;
          }
    }

  /* Update the button icon */
  if (G_LIKELY (pixbuf != NULL))
    xfce_panel_image_set_from_pixbuf (XFCE_PANEL_IMAGE (button->image), pixbuf);

  /* Update the tooltip */
  if (!button->is_configured)
    gtk_widget_set_tooltip_text (GTK_WIDGET (button), _("No valid device and/or element."));
  else
    {
      /* Set tooltip (e.g. 'Master: 50% (muted)') */
      if (button->is_muted)
        tip_text = g_strdup_printf (_("%s: muted"), button->track_label);
      else
        tip_text = g_strdup_printf (_("%s: %i%%"), button->track_label, (gint) round (value * 100));
      gtk_widget_set_tooltip_text (GTK_WIDGET (button), tip_text);
      g_free (tip_text);
    }
}



static void
xfce_volume_button_volume_changed (XfceVolumeButton *button,
                                   gdouble           volume)
{
  /* Do nothing */
}



void
xfce_volume_button_set_muted (XfceVolumeButton *button,
                              gboolean          is_muted)
{
  GValue value = G_VALUE_INIT;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, is_muted);
  g_object_set_property (G_OBJECT (button), "is-muted", &value);
}



gboolean
xfce_volume_button_get_muted (XfceVolumeButton *button)
{
  GValue value = G_VALUE_INIT;

  g_return_val_if_fail (IS_XFCE_VOLUME_BUTTON (button), FALSE);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (button), "is-muted", &value);

  return g_value_get_boolean (&value);
}



void 
xfce_volume_button_set_volume (XfceVolumeButton *button,
                               gdouble           volume)
{
  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  /* Set the value of the adjustment */
  gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), volume);

  /* Update the state of the button */
  xfce_volume_button_update (button);
}



static void
xfce_volume_button_update_icons (XfceVolumeButton *button,
                                 GtkIconTheme     *icon_theme)
{
  guint i;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));
  g_return_if_fail (GTK_IS_ICON_THEME (icon_theme));

  /* Pre-load all icons */
  for (i = 0; i < G_N_ELEMENTS (icons)-1; ++i)
    {
      if (GDK_IS_PIXBUF (button->pixbufs[i]))
        g_object_unref (G_OBJECT (button->pixbufs[i]));

      button->pixbufs[i] = gtk_icon_theme_load_icon (icon_theme,
                                                     icons[i],
                                                     button->icon_size,
                                                     GTK_ICON_LOOKUP_USE_BUILTIN,
                                                     NULL);
    }

  /* Update the state of the button */
  xfce_volume_button_update (button);
}



void
xfce_volume_button_set_icon_size (XfceVolumeButton *button,
                                  gint              size)
{
  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));
  g_return_if_fail (size >= 0);

  /* Remember the icon size */
  button->icon_size = size;

  xfce_volume_button_update_icons (button, gtk_icon_theme_get_default ());
}



void
xfce_volume_button_set_track_label (XfceVolumeButton *button,
                                    const gchar      *track_label)
{
  GValue value = G_VALUE_INIT;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, track_label);
  g_object_set_property (G_OBJECT (button), "track-label", &value);
}



gchar*
xfce_volume_button_get_track_label (XfceVolumeButton *button)
{
  GValue value = G_VALUE_INIT;

  g_return_val_if_fail (IS_XFCE_VOLUME_BUTTON (button), NULL);

  g_value_init (&value, G_TYPE_STRING);
  g_object_get_property (G_OBJECT (button), "track-label", &value);

  return g_value_dup_string (&value);
}



void
xfce_volume_button_set_is_configured (XfceVolumeButton *button,
                                      gboolean          is_configured)
{
  GValue value = G_VALUE_INIT;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, is_configured);
  g_object_set_property (G_OBJECT (button), "is-configured", &value);
}




gboolean
xfce_volume_button_get_is_configured (XfceVolumeButton *button)
{
  GValue value = G_VALUE_INIT;

  g_return_val_if_fail (IS_XFCE_VOLUME_BUTTON (button), FALSE);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (button), "is-configured", &value);

  return g_value_get_boolean (&value);
}
