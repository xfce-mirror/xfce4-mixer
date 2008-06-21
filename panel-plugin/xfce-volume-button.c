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

#include <gtk/gtk.h>

#include <gdk/gdkkeysyms.h>

#include <libxfcegui4/libxfcegui4.h>

#include "xfce-volume-button.h"



/* Signal identifiers */
enum
{
  VOLUME_CHANGED,
  LAST_SIGNAL,
};



static guint button_signals[LAST_SIGNAL];

static const char *icons[] = {
  "audio-volume-00",
  "audio-volume-01",
  "audio-volume-02",
  "audio-volume-03",
  "audio-volume-04",
  "audio-volume-05",
  NULL
};



static void       xfce_volume_button_class_init     (XfceVolumeButtonClass *klass);
static void       xfce_volume_button_init           (XfceVolumeButton      *button);
static void       xfce_volume_button_dispose        (GObject               *object);
static void       xfce_volume_button_finalize       (GObject               *object);
static void       xfce_volume_button_key_pressed    (GtkWidget             *widget,
                                                     GdkEventKey           *event,
                                                     XfceVolumeButton      *button);
static gboolean   xfce_volume_button_button_pressed (GtkWidget             *widget,
                                                     GdkEventButton        *event,
                                                     XfceVolumeButton      *button);
static void       xfce_volume_button_enter          (GtkWidget             *widget,
                                                     GdkEventCrossing      *event);
static void       xfce_volume_button_leave          (GtkWidget             *widget,
                                                     GdkEventCrossing      *event);
static void       xfce_volume_button_scrolled       (GtkWidget             *widget,
                                                     GdkEventScroll        *event,
                                                     XfceVolumeButton      *button);
static void       xfce_volume_button_volume_changed (XfceVolumeButton      *button,
                                                     gdouble                volume);



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

  GtkWidget *image;

  GtkObject *adjustment;

  gdouble    previous_value;

  gint       icon_size;
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
  button->adjustment = gtk_adjustment_new (0.0, 0.0, 1.0, 0.05, 0.05, 0.2);
  button->image = xfce_scaled_image_new ();

  gtk_container_add (GTK_CONTAINER (button), button->image);
  gtk_widget_show (button->image);

  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);

  g_signal_connect (G_OBJECT (button), "key-press-event", G_CALLBACK (xfce_volume_button_key_pressed), button);
  g_signal_connect (G_OBJECT (button), "button-press-event", G_CALLBACK (xfce_volume_button_button_pressed), button);
  g_signal_connect (G_OBJECT (button), "enter-notify-event", G_CALLBACK (xfce_volume_button_enter), NULL);
  g_signal_connect (G_OBJECT (button), "leave-notify-event", G_CALLBACK (xfce_volume_button_leave), NULL);
  g_signal_connect (G_OBJECT (button), "scroll-event", G_CALLBACK (xfce_volume_button_scrolled), button);

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
  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (object);

  (*G_OBJECT_CLASS (xfce_volume_button_parent_class)->finalize) (object);
}



GtkWidget*
xfce_volume_button_new (void)
{
  return g_object_new (TYPE_XFCE_VOLUME_BUTTON, NULL);
}



static void 
xfce_volume_button_key_pressed (GtkWidget        *widget,
                                GdkEventKey      *event,
                                XfceVolumeButton *button)
{
  gdouble value;
  gdouble step_increment;
  gdouble page_size;
  gdouble min_value;
  gdouble max_value;

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
        break;
      case GDK_minus:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value - step_increment);
        break;
      case GDK_Page_Up:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value + page_size);
        break;
      case GDK_Page_Down:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value - page_size);
        break;
      case GDK_Home:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), max_value);
        break;
      case GDK_End:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), min_value);
        break;
    }

  button->previous_value = value;

  xfce_volume_button_update (button);

  g_signal_emit_by_name (button, "volume-changed", gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment)));
}



static gboolean 
xfce_volume_button_button_pressed (GtkWidget        *widget,
                                   GdkEventButton   *event,
                                   XfceVolumeButton *button)
{
  gboolean handled = FALSE;
  gdouble value;
  gdouble min_value;

  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  g_object_get (G_OBJECT (button->adjustment), "value", &value, "lower", &min_value, NULL);

  if (event->button == 2)
    {
      if (gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment)) == min_value)
        {
          gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), button->previous_value);
          button->previous_value = value;
        }
      else
        {
          gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), min_value);
          button->previous_value = value;
        }

      handled = TRUE;
    }

  xfce_volume_button_update (button);

  g_signal_emit_by_name (button, "volume-changed", gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment)));

  return handled;
}



static void 
xfce_volume_button_enter (GtkWidget        *widget,
                          GdkEventCrossing *event)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (!GTK_WIDGET_HAS_FOCUS (widget))
    {
      gtk_widget_grab_focus (widget);
      gtk_grab_add (widget);
      gdk_pointer_grab (widget->window, TRUE, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK|GDK_SCROLL_MASK, NULL, NULL, GDK_CURRENT_TIME);
      gdk_keyboard_grab (widget->window, TRUE, GDK_CURRENT_TIME);
      gtk_widget_set_state (widget, GTK_STATE_SELECTED);
    }
}



static void 
xfce_volume_button_leave (GtkWidget        *widget,
                          GdkEventCrossing *event)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (GTK_WIDGET_HAS_FOCUS (widget))
    {
      gtk_widget_set_state (widget, GTK_STATE_NORMAL);
      gdk_keyboard_ungrab (GDK_CURRENT_TIME);
      gdk_pointer_ungrab (GDK_CURRENT_TIME);
      gtk_grab_remove (widget);
    }
}



static void 
xfce_volume_button_scrolled (GtkWidget        *widget,
                             GdkEventScroll   *event,
                             XfceVolumeButton *button)
{
  gdouble value;
  gdouble step_increment;

  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));

  g_object_get (G_OBJECT (button->adjustment), "value", &value, "step-increment", &step_increment, NULL);

  switch (event->direction)
    {
      case GDK_SCROLL_UP:
      case GDK_SCROLL_RIGHT:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value + step_increment);
        break;
      case GDK_SCROLL_DOWN:
      case GDK_SCROLL_LEFT:
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value - step_increment);
        break;
    }

  xfce_volume_button_update (button);

  g_signal_emit_by_name (button, "volume-changed", gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment)));
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

  g_object_get (G_OBJECT (button->adjustment), "upper", &upper, "lower", &lower, "value", &value, NULL);

  range = (upper - lower) / (G_N_ELEMENTS (icons) - 2);

  if (value == 0)
    pixbuf = xfce_themed_icon_load (icons[0], button->icon_size);
  else
    {
      for (i = 1; i < G_N_ELEMENTS (icons) - 1; ++i)
        if (value <= range * i)
          {
            pixbuf = xfce_themed_icon_load (icons[i], button->icon_size);
            break;
          }
    }

  if (G_LIKELY (pixbuf != NULL))
    {
      xfce_scaled_image_set_from_pixbuf (XFCE_SCALED_IMAGE (button->image), pixbuf);
      g_object_unref (G_OBJECT (pixbuf));
    }
}



static void
xfce_volume_button_volume_changed (XfceVolumeButton *button,
                                   gdouble           volume)
{
}



void 
xfce_volume_button_set_volume (XfceVolumeButton *button,
                               gdouble           volume)
{
  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));
  gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), volume);
  xfce_volume_button_update (button);
}


void
xfce_volume_button_set_icon_size (XfceVolumeButton *button,
                                  gint              size)
{
  g_return_if_fail (IS_XFCE_VOLUME_BUTTON (button));
  g_return_if_fail (size >= 0);
  button->icon_size = size;
}
