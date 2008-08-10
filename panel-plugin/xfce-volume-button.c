/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gdk/gdkkeysyms.h>

#include <libxfcegui4/libxfcegui4.h>

#include "libxfce4mixer/xfce-mixer-stock.h"

#include "xfce-volume-button.h"



/* Signal identifiers */
enum
{
  VOLUME_CHANGED,
  MUTE_TOGGLED,
  LAST_SIGNAL,
};



/* Signals */
static guint button_signals[LAST_SIGNAL];



/* Icons for different volume levels */
static const char *icons[] = {
  AUDIO_VOLUME_00,
  AUDIO_VOLUME_01,
  AUDIO_VOLUME_02,
  AUDIO_VOLUME_03,
  AUDIO_VOLUME_04,
  AUDIO_VOLUME_05,
  NULL
};



static void       xfce_volume_button_class_init     (XfceVolumeButtonClass *klass);
static void       xfce_volume_button_init           (XfceVolumeButton      *button);
static void       xfce_volume_button_dispose        (GObject               *object);
static void       xfce_volume_button_finalize       (GObject               *object);
#if 0
static gboolean   xfce_volume_button_key_pressed    (GtkWidget             *widget,
                                                     GdkEventKey           *event,
                                                     XfceVolumeButton      *button);
#endif
static gboolean   xfce_volume_button_button_pressed (GtkWidget             *widget,
                                                     GdkEventButton        *event,
                                                     XfceVolumeButton      *button);
static void       xfce_volume_button_enter          (GtkWidget             *widget,
                                                     GdkEventCrossing      *event);
static void       xfce_volume_button_leave          (GtkWidget             *widget,
                                                     GdkEventCrossing      *event);
static gboolean   xfce_volume_button_scrolled       (GtkWidget             *widget,
                                                     GdkEventScroll        *event,
                                                     XfceVolumeButton      *button);
static void       xfce_volume_button_volume_changed (XfceVolumeButton      *button,
                                                     gdouble                volume);
static void       xfce_volume_button_mute_toggled   (XfceVolumeButton      *button,
                                                     gboolean               mute);



struct _XfceVolumeButtonClass
{
  GtkButtonClass __parent__;

  /* Signals */
  void (*volume_changed) (XfceVolumeButton *button,
                          gdouble           volume);
  void (*mute_toggled)   (XfceVolumeButton *button,
                          gboolean          mute);
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

  klass->volume_changed = xfce_volume_button_volume_changed;
  klass->mute_toggled = xfce_volume_button_mute_toggled;

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

  button_signals[MUTE_TOGGLED] = g_signal_new ("mute-toggled",
                                               G_TYPE_FROM_CLASS (klass),
                                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                               G_STRUCT_OFFSET (XfceVolumeButtonClass, mute_toggled),
                                               NULL, 
                                               NULL,
                                               g_cclosure_marshal_VOID__BOOLEAN,
                                               G_TYPE_NONE, 
                                               1, 
                                               G_TYPE_BOOLEAN);
}



static void
xfce_volume_button_init (XfceVolumeButton *button)
{
  /* By default we expect the button not to be muted */
  button->is_muted = FALSE;

  /* Allocate array for preloaded icons */
  button->pixbufs = g_new0 (GdkPixbuf*, G_N_ELEMENTS (icons)-1);

  /* Create adjustment for the button (from 0.0 to 1.0 in 5% steps) */
  button->adjustment = gtk_adjustment_new (0.0, 0.0, 1.0, 0.05, 0.05, 0.2);

  /* Create a new scaled image for the button icon */
  button->image = xfce_scaled_image_new ();
  gtk_container_add (GTK_CONTAINER (button), button->image);
  gtk_widget_show (button->image);

  /* Make the button look flat and make it never grab the focus */
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (button), GTK_CAN_DEFAULT|GTK_CAN_FOCUS);

  /* Connect to button signals */
#if 0
  /* UNSED FOR NOW DUE TO TOO MUCH PROBLEMS WITH KEYBOARD FOCUS GRABBING */
  g_signal_connect (G_OBJECT (button), "key-press-event", G_CALLBACK (xfce_volume_button_key_pressed), button);
#endif
  g_signal_connect (G_OBJECT (button), "button-press-event", G_CALLBACK (xfce_volume_button_button_pressed), button);
  g_signal_connect (G_OBJECT (button), "scroll-event", G_CALLBACK (xfce_volume_button_scrolled), button);

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
  gint i;

  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (object);

  /* Free pre-allocated icon pixbufs */
  for (i = 0; i < G_N_ELEMENTS (icons)-1; ++i)
    if (GDK_IS_PIXBUF (button->pixbufs[i]))
      g_object_unref (G_OBJECT (button->pixbufs[i]));
  g_free (button->pixbufs);

  (*G_OBJECT_CLASS (xfce_volume_button_parent_class)->finalize) (object);
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

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  /* Check if the middle mouse button was pressed */
  if (event->button == 2)
    {
      /* Determine the new mute state by negating the current state */
      mute = !button->is_muted;

      /* Toggle the button's mute state */
      xfce_volume_button_set_muted (button, mute);

      /* Notify listeners of the mute change */
      g_signal_emit_by_name (button, "mute-toggled", mute);

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
  gdouble value;
  gdouble step_increment;

  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  /* Get current adjustment value and the step increment size */
  g_object_get (G_OBJECT (button->adjustment), "value", &value, "step-increment", &step_increment, NULL);

  /* Distinguish between scroll directions */
  switch (event->direction)
    {
      case GDK_SCROLL_UP:
      case GDK_SCROLL_RIGHT:
        /* Increase one step when scrolling up/right */
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value + step_increment);
        break;
      case GDK_SCROLL_DOWN:
      case GDK_SCROLL_LEFT:
        /* Decrease one step when scrolling down/left */
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value - step_increment);
        break;
    }

  /* Update the state of the button */
  xfce_volume_button_update (button);

  /* Notify listeners of the new volume */
  g_signal_emit_by_name (button, "volume-changed", gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment)));

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
  gint       i;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  /* Get upper, lower bounds and current value of the adjustment */
  g_object_get (G_OBJECT (button->adjustment), "upper", &upper, "lower", &lower, "value", &value, NULL);

  /* Determine the difference between upper and lower bound (= volume range) */
  range = (upper - lower) / (G_N_ELEMENTS (icons) - 2);

  if (G_UNLIKELY (button->is_muted))
    {
      /* By definition, use the first icon if the button is muted */
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
    xfce_scaled_image_set_from_pixbuf (XFCE_SCALED_IMAGE (button->image), pixbuf);
}



static void
xfce_volume_button_volume_changed (XfceVolumeButton *button,
                                   gdouble           volume)
{
  /* Do nothing */
}



void
xfce_volume_button_set_muted (XfceVolumeButton *button,
                              gboolean          muted)
{
  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  /* Change mute value */
  button->is_muted = muted;

  /* Update the state of the button */
  xfce_volume_button_update (button);
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


void
xfce_volume_button_set_icon_size (XfceVolumeButton *button,
                                  gint              size)
{
  gint i;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));
  g_return_if_fail (size >= 0);

  /* Remember the icon size */
  button->icon_size = size;

  /* Pre-load all icons */
  for (i = 0; i < G_N_ELEMENTS (icons)-1; ++i)
    {
      if (GDK_IS_PIXBUF (button->pixbufs[i]))
        g_object_unref (G_OBJECT (button->pixbufs[i]));

      button->pixbufs[i] = xfce_themed_icon_load (icons[i], button->icon_size);
    }
}



static void
xfce_volume_button_mute_toggled (XfceVolumeButton *button,
                                 gboolean          mute)
{
  /* Do nothing */
}
