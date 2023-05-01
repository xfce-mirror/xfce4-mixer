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

#include "xfce-volume-button.h"



#define SCALE_SIZE 128
#define VOLUME_EPSILON 0.005



/* Properties */
enum
{
  PROP_0,
  PROP_TRACK_LABEL,
  PROP_IS_CONFIGURED,
  PROP_NO_MUTE,
  PROP_IS_MUTED,
  PROP_SCREEN_POSITION,
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



static void       xfce_volume_button_dispose              (GObject               *object);
static void       xfce_volume_button_finalize             (GObject               *object);
static void       xfce_volume_button_set_property         (GObject               *object,
                                                           guint                  prop_id,
                                                           const GValue          *value,
                                                           GParamSpec            *pspec);
static void       xfce_volume_button_get_property         (GObject               *object,
                                                           guint                  prop_id,
                                                           GValue                *value,
                                                           GParamSpec            *pspec);
static gboolean   xfce_volume_button_scale_changed_value  (XfceVolumeButton      *button,
                                                           GtkScrollType          scroll,
                                                           gdouble                new_value,
                                                           GtkRange              *range);
static void       xfce_volume_button_create_dock_contents (XfceVolumeButton      *button);
static void       xfce_volume_button_popup_dock           (XfceVolumeButton      *button);
static void       xfce_volume_button_popdown_dock         (XfceVolumeButton      *button);
static gboolean   xfce_volume_button_button_press_event   (GtkWidget             *widget,
                                                           GdkEventButton        *event);
static gboolean   xfce_volume_button_scroll_event         (GtkWidget             *widget,
                                                           GdkEventScroll        *event);
static void       xfce_volume_button_update_icons         (XfceVolumeButton      *button,
                                                           GtkIconTheme          *icon_theme);
static void       xfce_volume_button_toggled              (GtkToggleButton       *toggle_button);
static gboolean   xfce_volume_button_dock_button_press    (XfceVolumeButton      *button,
                                                           GdkEventButton        *event,
                                                           GtkWidget             *widget);
static gboolean   xfce_volume_button_dock_key_release     (XfceVolumeButton      *button,
                                                           GdkEventKey           *event,
                                                           GtkWidget             *widget);
static void       xfce_volume_button_dock_grab_notify     (XfceVolumeButton      *button,
                                                           gboolean               was_grabbed,
                                                           GtkWidget             *widget);
static gboolean   xfce_volume_button_dock_grab_broken     (XfceVolumeButton      *button,
                                                           gboolean               was_grabbed,
                                                           GtkWidget             *widget);



struct _XfceVolumeButton
{
  GtkToggleButton      __parent__;

  /* Position of the dock */
  XfceScreenPosition   screen_position;

  /* Image widget for the volume icon */
  GtkWidget           *image;

  /* Popup window containing the scale */
  GtkWidget           *dock;

  /* Containers for the widgets inside the dock */
  GtkWidget           *hbox;
  GtkWidget           *vbox;

  /* Adjustment for the volume range and current value */
  GtkAdjustment       *adjustment;

  /* Icon size currently used */
  gint                 icon_size;

  /* Array of preloaded icons */
  GdkPixbuf          **pixbufs;

  /* Track label used in tooltip */
  gchar               *track_label;

  /* Whether the button is configured */
  gboolean             is_configured;

  /* Whether mute can be used */
  gboolean             no_mute;

  /* Mute state of the button */
  gboolean             is_muted;
};



G_DEFINE_TYPE (XfceVolumeButton, xfce_volume_button, GTK_TYPE_TOGGLE_BUTTON)



static void
xfce_volume_button_class_init (XfceVolumeButtonClass *klass)
{
  GObjectClass         *gobject_class;
  GtkWidgetClass       *gtk_widget_class;
  GtkToggleButtonClass *gtk_toggle_button_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_volume_button_dispose;
  gobject_class->finalize = xfce_volume_button_finalize;
  gobject_class->set_property = xfce_volume_button_set_property;
  gobject_class->get_property = xfce_volume_button_get_property;

  gtk_widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class->button_press_event = xfce_volume_button_button_press_event;
  gtk_widget_class->scroll_event = xfce_volume_button_scroll_event;

  gtk_toggle_button_class = GTK_TOGGLE_BUTTON_CLASS (klass);
  gtk_toggle_button_class->toggled = xfce_volume_button_toggled;

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
                                   PROP_NO_MUTE,
                                   g_param_spec_boolean ("no-mute",
                                                         "no-mute",
                                                         "no-mute",
                                                         TRUE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));


  g_object_class_install_property (gobject_class,
                                   PROP_IS_MUTED,
                                   g_param_spec_boolean ("is-muted",
                                                         "is-muted",
                                                         "is-muted",
                                                         TRUE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_SCREEN_POSITION,
                                   g_param_spec_enum ("screen-position",
                                                      "screen-position",
                                                      "screen-position",
                                                      XFCE_TYPE_SCREEN_POSITION,
                                                      XFCE_SCREEN_POSITION_FLOATING_H,
                                                      G_PARAM_READABLE | G_PARAM_WRITABLE));

  button_signals[VOLUME_CHANGED] = g_signal_new ("volume-changed",
                                                 G_TYPE_FROM_CLASS (klass),
                                                 G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                                 0,
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
  /* The dock is created lazily */
  button->dock = NULL;
  button->hbox = NULL;
  button->vbox = NULL;

  button->track_label = NULL;

  /* Start in unconfigured state */
  button->is_configured = FALSE;

  /* Default position is floating horizontal */
  button->screen_position = XFCE_SCREEN_POSITION_FLOATING_H;

  /* Allocate array for preloaded icons */
  button->pixbufs = g_new0 (GdkPixbuf*, G_N_ELEMENTS (icons)-1);

  /* Create adjustment for the button (from 0.0 to 1.0 in 5% steps) */
  button->adjustment = gtk_adjustment_new (0.0, 0.0, 1.0, 0.01, 0.05, 0.0);

  /* By default mute can be used */
  button->no_mute = FALSE;

  /* Set to muted by default since the initial adjustment value is 0 */
  button->is_muted = TRUE;

  /* Create a new scaled image for the button icon */
  button->image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (button), button->image);
  gtk_widget_show (button->image);

  /* Make the button look flat and make it never grab the focus */
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_set_focus_on_click (GTK_WIDGET (button), FALSE);
  gtk_widget_set_can_default (GTK_WIDGET (button), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (button), FALSE);

  /* Connect signal for theme changes */
  g_signal_connect_swapped (gtk_icon_theme_get_default (), "changed", G_CALLBACK (xfce_volume_button_update_icons), button);

  /* Intercept scroll events */
  gtk_widget_add_events (GTK_WIDGET (button), GDK_SCROLL_MASK);

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

  if (button->dock != NULL)
    {
      gtk_widget_destroy (button->dock);
      button->dock = NULL;
    }

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
  gboolean          no_mute;
  gboolean          is_muted;

  switch (prop_id)
    {
      case PROP_TRACK_LABEL:
        g_free (button->track_label);
        button->track_label = g_value_dup_string (value);
        if (button->is_configured)
          xfce_volume_button_update (button);
        break;
      case PROP_NO_MUTE:
        no_mute = g_value_get_boolean (value);
        if (button->is_configured && button->no_mute != no_mute)
          {
            button->no_mute = no_mute;
            if (no_mute)
              button->is_muted = FALSE;
            xfce_volume_button_update (button);
          }
        break;
      case PROP_IS_MUTED:
        is_muted = g_value_get_boolean (value);
        if (button->is_configured && !button->no_mute && button->is_muted != is_muted)
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

            /* Popdown the dock when transitioning to unconfigured state */
            if (is_configured == FALSE &&
                (button->dock != NULL && gtk_widget_get_visible (button->dock)))
              xfce_volume_button_popdown_dock (button);

            xfce_volume_button_update (button);
          }
        break;
      case PROP_SCREEN_POSITION:
        button->screen_position = g_value_get_enum (value);
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
      case PROP_NO_MUTE:
        g_value_set_boolean (value, button->no_mute);
        break;
      case PROP_IS_MUTED:
        g_value_set_boolean (value, button->is_muted);
        break;
      case PROP_IS_CONFIGURED:
        g_value_set_boolean (value, button->is_configured);
        break;
      case PROP_SCREEN_POSITION:
        g_value_set_enum (value, button->screen_position);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}



GtkWidget*
xfce_volume_button_new (void)
{
  return g_object_new (XFCE_TYPE_VOLUME_BUTTON, NULL);
}



static gboolean
xfce_volume_button_scale_changed_value (XfceVolumeButton *button,
                                        GtkScrollType     scroll,
                                        gdouble           value,
                                        GtkRange         *range)
{
  gdouble old_value;
  gdouble new_value;

  old_value = gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment));
  gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), value);
  new_value = gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment));

  if (fabs (new_value - old_value) > VOLUME_EPSILON)
    {
      /* Update the state of the button */
      xfce_volume_button_update (button);

      /* Notify listeners of the new volume */
      g_signal_emit_by_name (button, "volume-changed", new_value);
    }

  /* Do not propagate further, everything has been handled */
  return TRUE;
}



static void
xfce_volume_button_create_dock_contents (XfceVolumeButton *button)
{
  GtkOrientation   orientation;
  GtkWidget       *frame;
  GtkWidget       *box;
  GtkWidget       *scale;
  GtkWidget       *image;

  g_return_if_fail (button->dock == NULL);

  orientation = xfce_screen_position_get_orientation (button->screen_position);

  button->dock = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_title (GTK_WINDOW (button->dock), "xfce4-mixer-applet-dock-window");
  gtk_window_set_decorated (GTK_WINDOW (button->dock), FALSE);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (button->dock), frame);
  gtk_widget_show (frame);

  /*
   * Container for the boxes for horizonal and vertical mode, orientation does
   * not matter here since only one of the boxes it holds will be visibe at any
   * time depending on the panel orientation
   */
  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (box), 2);
  gtk_container_add (GTK_CONTAINER (frame), box);
  gtk_widget_show (box);

  /* Container for the widgets shown in vertical mode */
  button->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (box), button->hbox, TRUE, TRUE, 0);

  /* Show the position of lowest and highest volume through icons */
  image = gtk_image_new_from_icon_name ("audio-volume-low", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (button->hbox), image, TRUE, TRUE, 0);
  gtk_widget_show (image);

  scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (button->adjustment));
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_box_pack_start (GTK_BOX (button->hbox), scale, TRUE, TRUE, 0);
  gtk_widget_set_size_request (scale, SCALE_SIZE, -1);
  g_signal_connect_swapped (G_OBJECT (scale), "change-value", G_CALLBACK (xfce_volume_button_scale_changed_value), button);
  gtk_widget_show (scale);

  image = gtk_image_new_from_icon_name ("audio-volume-high", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (button->hbox), image, TRUE, TRUE, 0);
  gtk_widget_show (image);

  /* Container for the widgets shown in horizontal mode */
  button->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (box), button->vbox, TRUE, TRUE, 0);

  scale = gtk_scale_new (GTK_ORIENTATION_VERTICAL, GTK_ADJUSTMENT (button->adjustment));
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_range_set_inverted (GTK_RANGE (scale), TRUE);
  gtk_box_pack_start (GTK_BOX (button->vbox), scale, TRUE, TRUE, 0);
  gtk_widget_set_size_request (scale, -1, SCALE_SIZE);
  g_signal_connect_swapped (G_OBJECT (scale), "change-value", G_CALLBACK (xfce_volume_button_scale_changed_value), button);
  gtk_widget_show (scale);

  if (orientation == GTK_ORIENTATION_VERTICAL)
    gtk_widget_show (button->hbox);
  else
    gtk_widget_show (button->vbox);

  g_signal_connect_swapped (G_OBJECT (button->dock), "button-press-event", G_CALLBACK (xfce_volume_button_dock_button_press), button);
  g_signal_connect_swapped (G_OBJECT (button->dock), "key-release-event", G_CALLBACK (xfce_volume_button_dock_key_release), button);
  g_signal_connect_swapped (G_OBJECT (button->dock), "grab-notify", G_CALLBACK (xfce_volume_button_dock_grab_notify), button);
  g_signal_connect_swapped (G_OBJECT (button->dock), "grab-broken-event", G_CALLBACK (xfce_volume_button_dock_grab_broken), button);
}



static gboolean
xfce_volume_button_grab_input (XfceVolumeButton *button)
{
  GtkWidget        *dock = button->dock;
  GdkWindow        *window = gtk_widget_get_window (dock);
  GdkDisplay       *display = gtk_widget_get_display (dock);
  GdkSeat          *seat = gdk_display_get_default_seat (display);

  gtk_grab_add (dock);

  if (gdk_seat_grab (seat, window, GDK_SEAT_CAPABILITY_ALL, TRUE, NULL, NULL, NULL, NULL) != GDK_GRAB_SUCCESS)
    goto fail_remove_grab;

  return TRUE;

fail_remove_grab:
  gtk_grab_remove (dock);
  return FALSE;
}



static void
xfce_volume_button_ungrab_input (XfceVolumeButton *button)
{
  GtkWidget        *dock = button->dock;
  GdkDisplay       *display = gtk_widget_get_display (dock);
  GdkSeat          *seat = gdk_display_get_default_seat (display);

  gdk_seat_ungrab (seat);

  gtk_grab_remove (dock);
}



static void
xfce_volume_button_popup_dock (XfceVolumeButton *button)
{
  GtkWidget       *button_widget = GTK_WIDGET (button);
  GtkOrientation   orientation;
  GtkRequisition   dock_requisition;
  GdkDisplay      *display;
  GdkMonitor      *gdkmonitor;
  GdkRectangle     monitor;
  gint             window_x;
  gint             window_y;
  GdkWindow       *window;
  gint             x;
  gint             y;
  GtkPositionType  position;
  GtkAllocation    button_allocation;

  /* Lazily create dock contents */
  if (button->dock == NULL)
    xfce_volume_button_create_dock_contents (button);

  /* Change orientation if necessary */
  orientation = xfce_screen_position_get_orientation (button->screen_position);
  if ((gtk_widget_get_visible (button->hbox) && orientation != GTK_ORIENTATION_VERTICAL) ||
      (gtk_widget_get_visible (button->vbox) && orientation != GTK_ORIENTATION_HORIZONTAL))
    {
      if (orientation == GTK_ORIENTATION_VERTICAL)
        {
          gtk_widget_hide (button->vbox);
          gtk_widget_show (button->hbox);
        }
      else
        {
          gtk_widget_hide (button->hbox);
          gtk_widget_show (button->vbox);
        }

      /* Hack to prevent window from becoming square */
      gtk_window_resize (GTK_WINDOW (button->dock), 1, 1);
    }

  /* Get size request of the dock */
  gtk_widget_get_preferred_size (GTK_WIDGET (button->dock), NULL, &dock_requisition);

  /* Determine the absolute coordinates of the button widget */
  gdk_window_get_origin (gtk_widget_get_window (GTK_WIDGET (button)), &x, &y);
  gtk_widget_get_allocation (button_widget, &button_allocation);
  x += button_allocation.x;
  y += button_allocation.y;

  /* Determine the geometry of the monitor containing the window containing the button */
  display = gtk_widget_get_display (button_widget);
  window = gtk_widget_get_window (GTK_WIDGET (button));
  gdkmonitor = gdk_display_get_monitor_at_window (display, window);
  gdk_monitor_get_geometry (gdkmonitor, &monitor);

  /* Determine the position of the window containing the button */
  if (xfce_screen_position_is_top (button->screen_position))
    position = GTK_POS_BOTTOM;
  else if (xfce_screen_position_is_bottom (button->screen_position))
    position = GTK_POS_TOP;
  else if (xfce_screen_position_is_left (button->screen_position))
    position = GTK_POS_RIGHT;
  else if (xfce_screen_position_is_right (button->screen_position))
    position = GTK_POS_LEFT;
  else
    {
      /*
       * If the window containing the button is floating derive a position
       * based on the the closest monitor edge
       */
      gdk_window_get_root_origin (window, &window_x, &window_y);

      if (button->screen_position == XFCE_SCREEN_POSITION_FLOATING_V)
        position = (window_x < (monitor.x + monitor.width / 2)) ? GTK_POS_RIGHT : GTK_POS_LEFT;
      else
        position = (window_y < (monitor.y + monitor.height / 2)) ? GTK_POS_BOTTOM : GTK_POS_TOP;
    }

  /* Place the dock centered on the correct edge of the button */
  switch (position)
    {
      case GTK_POS_TOP:
        x += (button_allocation.width / 2) - (dock_requisition.width / 2);
        y -= dock_requisition.height;
        break;
      case GTK_POS_RIGHT:
        x += button_allocation.width;
        y += (button_allocation.height / 2) - (dock_requisition.height / 2);
        break;
      case GTK_POS_LEFT:
        x -= dock_requisition.width;
        y += (button_allocation.height / 2) - (dock_requisition.height / 2);
        break;
      case GTK_POS_BOTTOM:
      default:
        /* default to GTK_POS_BOTTOM */
        x += (button_allocation.width / 2) - (dock_requisition.width / 2);
        y += button_allocation.height;
        break;
    }

  /* Ensure the dock remains on the monitor */
  if (x > monitor.x + monitor.width - dock_requisition.width)
    x = monitor.x + monitor.width - dock_requisition.width;
  if (x < monitor.x)
    x = monitor.x;
  if (y > monitor.y + monitor.height - dock_requisition.height)
    y = monitor.y + monitor.height - dock_requisition.height;
  if (y < monitor.y)
    y = monitor.y;

  /* Position the dock */
  gtk_window_move (GTK_WINDOW (button->dock), x, y);

  gtk_widget_show (button->dock);

  /* Grab keyboard and mouse, focus on the slider */
  if (!xfce_volume_button_grab_input (button))
    {
      gtk_widget_hide (button->dock);
      return;
    }

  gtk_widget_grab_focus (button->dock);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
}



static void
xfce_volume_button_popdown_dock (XfceVolumeButton *button)
{
  if (button->dock != NULL && gtk_widget_get_visible (button->dock))
    {
      xfce_volume_button_ungrab_input (button);

      gtk_widget_hide (button->dock);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
    }
}



static gboolean
xfce_volume_button_button_press_event (GtkWidget      *widget,
                                       GdkEventButton *event)
{
  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (widget);
  gboolean          muted;

  if (event->button == 1)
    {
      if ((button->dock == NULL || !gtk_widget_get_visible (GTK_WIDGET (button->dock))) &&
          !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
        xfce_volume_button_popup_dock (button);

      return TRUE;
    }
  else if (event->button == 2)
    {
      /* Only toggle mute if button is in configured state and can be muted */
      if (button->is_configured && !button->no_mute)
        {
          /* Determine the new mute state by negating the current state */
          muted = !button->is_muted;

          /* Toggle the button's mute state */
          xfce_volume_button_set_muted (button, muted);
        }

      /* Middle mouse button was handled, do not propagate the event any further */
      return TRUE;
    }

  return GTK_WIDGET_CLASS (xfce_volume_button_parent_class)->button_press_event (widget, event);
}



static gboolean
xfce_volume_button_scroll_event (GtkWidget      *widget,
                                 GdkEventScroll *event)
{
  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (widget);
  gdouble           old_value;
  gdouble           new_value;
  gdouble           increment;

  /* Ignore scroll events if the button is not in configured state */
  if (!button->is_configured)
    return TRUE;

  /* Get current adjustment value and the step increment size */
  g_object_get (G_OBJECT (button->adjustment), "value", &old_value, "page-increment", &increment, NULL);

  /* Distinguish between scroll directions */
  switch (event->direction)
    {
      case GDK_SCROLL_UP:
      case GDK_SCROLL_RIGHT:
        /* Increase one step when scrolling up/right */
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), old_value + increment);
        break;
      case GDK_SCROLL_DOWN:
      case GDK_SCROLL_LEFT:
        /* Decrease one step when scrolling down/left */
        gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), old_value - increment);
        break;

      case GDK_SCROLL_SMOOTH:
        break;
    }

  new_value = gtk_adjustment_get_value (GTK_ADJUSTMENT (button->adjustment));
  if (fabs (new_value - old_value) > VOLUME_EPSILON)
    {
      /* Update the state of the button */
      xfce_volume_button_update (button);

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

  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));

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
    gtk_image_set_from_pixbuf (GTK_IMAGE (button->image), pixbuf);

  /* Update the tooltip */
  if (!button->is_configured)
    gtk_widget_set_tooltip_text (GTK_WIDGET (button), _("No valid device and/or element."));
  else
    {
      /* Set tooltip (e.g. 'Master: 50% (muted)') */
      if (button->is_muted && !button->no_mute)
        tip_text = g_strdup_printf (_("%s: muted"), button->track_label);
      else
        tip_text = g_strdup_printf (_("%s: %i%%"), button->track_label, (gint) round (value * 100));
      gtk_widget_set_tooltip_text (GTK_WIDGET (button), tip_text);
      g_free (tip_text);
    }
}



static void
xfce_volume_button_update_icons (XfceVolumeButton *button,
                                 GtkIconTheme     *icon_theme)
{
  guint i;

  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));
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



static void
xfce_volume_button_toggled (GtkToggleButton *toggle_button)
{
  XfceVolumeButton *button = XFCE_VOLUME_BUTTON (toggle_button);

  if (gtk_toggle_button_get_active (toggle_button) &&
      (button->dock == NULL || !gtk_widget_get_visible (GTK_WIDGET (button->dock))))
    xfce_volume_button_popup_dock (button);
}



static gboolean
xfce_volume_button_dock_button_press (XfceVolumeButton *button,
                                      GdkEventButton   *event,
                                      GtkWidget        *widget)
{
  /* Pop down on mouse button click */
  if (event->type == GDK_BUTTON_PRESS)
    {
      xfce_volume_button_popdown_dock (button);

      return TRUE;
    }

  return FALSE;
}



static gboolean
xfce_volume_button_dock_key_release (XfceVolumeButton *button,
                                     GdkEventKey      *event,
                                     GtkWidget        *widget)
{
  /* Pop down on Escape */
  if (event->keyval == GDK_KEY_Escape)
    {
      xfce_volume_button_popdown_dock (button);
      return TRUE;
    }

  return TRUE;
}



static void
xfce_volume_button_dock_grab_notify (XfceVolumeButton *button,
                                     gboolean          was_grabbed,
                                     GtkWidget        *widget)
{
  /* Pop down if button->dock has been shadowed by a grab on another widget */
  if (!was_grabbed &&
      gtk_widget_has_grab (button->dock) &&
      !gtk_widget_is_ancestor (gtk_grab_get_current (), button->dock))
    xfce_volume_button_popdown_dock (button);
}



static gboolean
xfce_volume_button_dock_grab_broken (XfceVolumeButton *button,
                                     gboolean          was_grabbed,
                                     GtkWidget        *widget)
{
  /* Pop down if grab is broken but not when grabbing again */
  if (gtk_widget_has_grab (button->dock) &&
      !gtk_widget_is_ancestor (gtk_grab_get_current (), button->dock))
    xfce_volume_button_popdown_dock (button);

  return FALSE;
}



void
xfce_volume_button_set_no_mute (XfceVolumeButton *button,
                                gboolean          no_mute)
{
  GValue value = { 0 };

  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, no_mute);
  g_object_set_property (G_OBJECT (button), "no-mute", &value);
}



gboolean
xfce_volume_button_get_no_mute (XfceVolumeButton *button)
{
  GValue value = { 0 };

  g_return_val_if_fail (XFCE_IS_VOLUME_BUTTON (button), FALSE);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (button), "no-mute", &value);

  return g_value_get_boolean (&value);
}



void
xfce_volume_button_set_muted (XfceVolumeButton *button,
                              gboolean          is_muted)
{
  GValue value = { 0 };

  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, is_muted);
  g_object_set_property (G_OBJECT (button), "is-muted", &value);
}



gboolean
xfce_volume_button_get_muted (XfceVolumeButton *button)
{
  GValue value = { 0 };

  g_return_val_if_fail (XFCE_IS_VOLUME_BUTTON (button), FALSE);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (button), "is-muted", &value);

  return g_value_get_boolean (&value);
}



void 
xfce_volume_button_set_volume (XfceVolumeButton *button,
                               gdouble           volume)
{
  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));

  /* Set the value of the adjustment */
  gtk_adjustment_set_value (GTK_ADJUSTMENT (button->adjustment), volume);

  /* Update the state of the button */
  xfce_volume_button_update (button);
}



void
xfce_volume_button_set_icon_size (XfceVolumeButton *button,
                                  gint              size)
{
  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));
  g_return_if_fail (size >= 0);

  /* Remember the icon size */
  button->icon_size = size;

  xfce_volume_button_update_icons (button, gtk_icon_theme_get_default ());
}



void
xfce_volume_button_set_track_label (XfceVolumeButton *button,
                              const gchar      *track_label)
{
  GValue value = { 0 };

  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, track_label);
  g_object_set_property (G_OBJECT (button), "track-label", &value);
}



gchar*
xfce_volume_button_get_track_label (XfceVolumeButton *button)
{
  GValue value = { 0 };

  g_return_val_if_fail (XFCE_IS_VOLUME_BUTTON (button), NULL);

  g_value_init (&value, G_TYPE_STRING);
  g_object_get_property (G_OBJECT (button), "track-label", &value);

  return g_value_dup_string (&value);
}



void
xfce_volume_button_set_is_configured (XfceVolumeButton *button,
                                      gboolean          is_configured)
{
  GValue value = { 0 };

  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, is_configured);
  g_object_set_property (G_OBJECT (button), "is-configured", &value);
}



gboolean
xfce_volume_button_get_is_configured (XfceVolumeButton *button)
{
  GValue value = { 0 };

  g_return_val_if_fail (XFCE_IS_VOLUME_BUTTON (button), FALSE);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (button), "is-configured", &value);

  return g_value_get_boolean (&value);
}



void
xfce_volume_button_set_screen_position (XfceVolumeButton   *button,
                                        XfceScreenPosition  screen_position)
{
  GValue value = { 0 };

  g_return_if_fail (XFCE_IS_VOLUME_BUTTON (button));

  g_value_init (&value, XFCE_TYPE_SCREEN_POSITION);
  g_value_set_enum (&value, screen_position);
  g_object_set_property (G_OBJECT (button), "screen-position", &value);
}



XfceScreenPosition
xfce_volume_button_get_screen_position (XfceVolumeButton *button)
{
  GValue value = { 0 };

  g_return_val_if_fail (XFCE_IS_VOLUME_BUTTON (button), FALSE);

  g_value_init (&value, XFCE_TYPE_SCREEN_POSITION);
  g_object_get_property (G_OBJECT (button), "screen-position", &value);

  return g_value_get_enum (&value);
}

