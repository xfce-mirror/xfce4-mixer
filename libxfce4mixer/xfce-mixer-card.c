/* $Id$ */
/* vim:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your card) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include "xfce-mixer-card.h"
#include "xfce-mixer-preferences.h"



static void xfce_mixer_card_class_init (XfceMixerCardClass *klass);
static void xfce_mixer_card_init       (XfceMixerCard      *card);
static void xfce_mixer_card_finalize   (GObject            *object);



struct _XfceMixerCardClass
{
  GObjectClass __parent__;
};

struct _XfceMixerCard
{
  GObject __parent__;

  GstElement *element;

#ifdef HAVE_GST_MIXER_NOTIFICATION
  GstBus     *bus;
#endif

  gchar      *display_name;
  gchar      *name;
};



static GObjectClass *xfce_mixer_card_parent_class = NULL;



GType
xfce_mixer_card_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerCardClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_card_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerCard),
          0,
          (GInstanceInitFunc) xfce_mixer_card_init,
          NULL,
        };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMixerCard", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_card_class_init (XfceMixerCardClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_card_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_mixer_card_finalize;
}



static void
xfce_mixer_card_init (XfceMixerCard *card)
{
  card->name = NULL;

#ifdef HAVE_GST_MIXER_NOTIFICATION
  card->bus = NULL;
#endif
}



static void
xfce_mixer_card_finalize (GObject *object)
{
  XfceMixerCard *card = XFCE_MIXER_CARD (object);

  gst_element_set_state (card->element, GST_STATE_NULL);
  gst_object_unref (card->element);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  if (G_LIKELY (card->bus != NULL))
    gst_object_unref (card->bus);
#endif

  g_free (card->display_name);
  g_free (card->name);

  (*G_OBJECT_CLASS (xfce_mixer_card_parent_class)->finalize) (object);
}



XfceMixerCard*
xfce_mixer_card_new (GstElement *element)
{
  XfceMixerCard *card;

  card = g_object_new (TYPE_XFCE_MIXER_CARD, NULL);
//  card->element = gst_object_ref (element);
  card->element = element;
  card->display_name = g_strdup (g_object_get_data (G_OBJECT (card->element), "xfce-mixer-control-name"));

  xfce_mixer_card_set_ready (card);

  return card;
}



const gchar*
xfce_mixer_card_get_display_name (XfceMixerCard *card)
{
  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), NULL);
  return card->display_name;
}



const gchar*
xfce_mixer_card_get_name (XfceMixerCard *card)
{
  gint   i;
  gint   pos = 0;

  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), NULL);

  if (G_UNLIKELY (card->name == NULL))
    {
      card->name = g_new (gchar, strlen (card->display_name));

      for (i = 0; card->display_name[i] != '\0'; ++i)
        if (g_ascii_isalnum (card->display_name[i]))
          card->name[pos++] = card->display_name[i];

      card->name[pos] = '\0';
    }

  return card->name;
}



void
xfce_mixer_card_set_ready (XfceMixerCard *card)
{
  g_return_if_fail (IS_XFCE_MIXER_CARD (card));
  
  gst_element_set_state (card->element, GST_STATE_READY);
  gst_element_get_state (card->element, NULL, NULL, GST_CLOCK_TIME_NONE);
}



GList*
xfce_mixer_card_get_visible_controls (XfceMixerCard *card)
{
  XfceMixerPreferences *preferences;
  GList                *list = NULL;

  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), NULL);

  preferences = xfce_mixer_preferences_get ();
  list = xfce_mixer_preferences_get_visible_controls (preferences, xfce_mixer_card_get_name (card));
  g_object_unref (G_OBJECT (preferences));

  return list;
}



const GList*
xfce_mixer_card_get_tracks (XfceMixerCard *card)
{
  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), NULL);
  return gst_mixer_list_tracks (GST_MIXER (card->element));
}



GstMixerTrack*
xfce_mixer_card_get_track_by_name (XfceMixerCard *card,
                                   const gchar   *track_name)
{
  GstMixerTrack *track = NULL;
  const GList   *tracks;
  const GList   *iter;

  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), NULL);
  g_return_val_if_fail (track_name != NULL, NULL);

  tracks = gst_mixer_list_tracks (GST_MIXER (card->element));

  for (iter = tracks; iter != NULL; iter = iter->next)
    if (G_UNLIKELY (g_utf8_collate (GST_MIXER_TRACK (iter->data)->label, track_name) == 0))
      {
        track = GST_MIXER_TRACK (iter->data);
        break;
      }

  return track;
}



void
xfce_mixer_card_set_control_visible (XfceMixerCard *card,
                                     const gchar   *control,
                                     gboolean       visible)
{
  XfceMixerPreferences *preferences;

  g_return_if_fail (IS_XFCE_MIXER_CARD (card));
  g_return_if_fail (control != NULL);

  preferences = xfce_mixer_preferences_get ();
  xfce_mixer_preferences_set_control_visible (preferences, xfce_mixer_card_get_name (card), control, visible);
  g_object_unref (G_OBJECT (preferences));
}



void
xfce_mixer_card_get_track_volume (XfceMixerCard *card,
                                  GstMixerTrack *track,
                                  gint          *volumes)
{
  g_return_if_fail (IS_XFCE_MIXER_CARD (card));
  g_return_if_fail (GST_IS_MIXER_TRACK (track));

  gst_mixer_get_volume (GST_MIXER (card->element), track, volumes);
}



void
xfce_mixer_card_set_track_volume (XfceMixerCard *card,
                                  GstMixerTrack *track,
                                  gint          *volumes)
{
  g_return_if_fail (IS_XFCE_MIXER_CARD (card));
  g_return_if_fail (GST_IS_MIXER_TRACK (track));

  gst_mixer_set_volume (GST_MIXER (card->element), track, volumes);
}



void
xfce_mixer_card_set_track_muted (XfceMixerCard *card,
                                 GstMixerTrack *track,
                                 gboolean       muted)
{
  g_return_if_fail (IS_XFCE_MIXER_CARD (card));
  g_return_if_fail (GST_IS_MIXER_TRACK (track));
  
  gst_mixer_set_mute (GST_MIXER (card->element), track, muted);
}



void
xfce_mixer_card_set_track_record (XfceMixerCard *card,
                                  GstMixerTrack *track,
                                  gboolean       record)
{
  g_return_if_fail (IS_XFCE_MIXER_CARD (card));
  g_return_if_fail (GST_IS_MIXER_TRACK (track));
  
  gst_mixer_set_record (GST_MIXER (card->element), track, record);
}



const gchar*
xfce_mixer_card_get_track_option (XfceMixerCard *card,
                                  GstMixerTrack *track)
{
  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), NULL);
  g_return_val_if_fail (GST_IS_MIXER_OPTIONS (track), NULL);

  return gst_mixer_get_option (GST_MIXER (card->element), GST_MIXER_OPTIONS (track));
}



void
xfce_mixer_card_set_track_option (XfceMixerCard *card,
                                  GstMixerTrack *track,
                                  gchar         *option)
{
  g_return_if_fail (IS_XFCE_MIXER_CARD (card));
  g_return_if_fail (GST_IS_MIXER_OPTIONS (track));

  gst_mixer_set_option (GST_MIXER (card->element), GST_MIXER_OPTIONS (track), option);
}



#ifdef HAVE_GST_MIXER_NOTIFICATION
void
xfce_mixer_card_connect (XfceMixerCard *card,
                         GCallback      callback_func,
                         gpointer       user_data)
{
  g_return_if_fail (IS_XFCE_MIXER_CARD (card));

  if (G_UNLIKELY (card->bus == NULL))
    {
      card->bus = gst_bus_new ();
      gst_bus_add_signal_watch (card->bus);
      gst_element_set_bus (card->element, card->bus);
    }

  g_signal_connect (G_OBJECT (card->bus), "message::element", callback_func, user_data);
}



gboolean
xfce_mixer_card_get_message_owner (XfceMixerCard *card,
                                   GstMessage    *message)
{
  gboolean  is_owner;
  gchar    *device_name1;
  gchar    *device_name2;

  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), FALSE);
  g_return_val_if_fail (GST_IS_MESSAGE (message), FALSE);

  if (!GST_IS_MIXER (GST_MESSAGE_SRC (message)))
    return FALSE;

  g_object_get (G_OBJECT (GST_MESSAGE_SRC (message)), "device-name", &device_name1, NULL);
  g_object_get (G_OBJECT (card->element), "device-name", &device_name2, NULL);

  is_owner = (GST_MESSAGE_SRC (message) == card->element || g_utf8_collate (device_name1, device_name2) == 0);

  g_free (device_name1);
  g_free (device_name2);

  return is_owner;
}
#endif
