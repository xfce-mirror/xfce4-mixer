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

#include <glib-object.h>

#include <libxfce4util/libxfce4util.h>
#include <xfconf/xfconf.h>

#include "libxfce4mixer.h"
#include "xfce-mixer-preferences.h"


enum
{
  PROP_0,
  PROP_WINDOW_WIDTH,
  PROP_WINDOW_HEIGHT,
  PROP_SOUND_CARD,
  PROP_CONTROLS,
  N_PROPERTIES,
};



static void       xfce_mixer_preferences_finalize           (GObject                   *object);
static void       xfce_mixer_preferences_get_property       (GObject                   *object,
                                                             guint                      prop_id,
                                                             GValue                    *value,
                                                             GParamSpec                *pspec);
static void       xfce_mixer_preferences_set_property       (GObject                   *object,
                                                             guint                      prop_id,
                                                             const GValue              *value,
                                                             GParamSpec                *pspec);
static GPtrArray *xfce_mixer_preferences_get_default_tracks (XfceMixerPreferences *preferences);



struct _XfceMixerPreferencesClass
{
  GObjectClass __parent__;
};

struct _XfceMixerPreferences
{
  GObject        __parent__;

  XfconfChannel *channel;

  gint           window_width;
  gint           window_height;

  gchar         *sound_card;

  GPtrArray     *controls;

  gulong         sound_cards_property_bind_id;
};



G_DEFINE_TYPE (XfceMixerPreferences, xfce_mixer_preferences, G_TYPE_OBJECT)



static void
xfce_mixer_preferences_class_init (XfceMixerPreferencesClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->get_property = xfce_mixer_preferences_get_property;
  gobject_class->set_property = xfce_mixer_preferences_set_property;
  gobject_class->finalize = xfce_mixer_preferences_finalize;

  g_object_class_install_property (gobject_class, 
                                   PROP_WINDOW_WIDTH,
                                   g_param_spec_int ("window-width",
                                                     "window-width",
                                                     "window-width",
                                                     1, G_MAXINT, 600,
                                                     G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, 
                                   PROP_WINDOW_HEIGHT,
                                   g_param_spec_int ("window-height",
                                                     "window-height",
                                                     "window-height",
                                                     1, G_MAXINT, 400,
                                                     G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, 
                                   PROP_SOUND_CARD,
                                   g_param_spec_string ("sound-card",
                                                        "sound-card",
                                                        "sound-card",
                                                        NULL,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, 
                                   PROP_CONTROLS,
                                   g_param_spec_boxed ("controls",
                                                       "controls",
                                                       "controls",
                                                       G_TYPE_PTR_ARRAY,
                                                       G_PARAM_READABLE | G_PARAM_WRITABLE));
}



static void
xfce_mixer_preferences_init (XfceMixerPreferences *preferences)
{
  preferences->channel = xfconf_channel_get ("xfce4-mixer");

  preferences->window_width = 640;
  preferences->window_height = 400;

  preferences->sound_card = NULL;

  preferences->controls = g_ptr_array_new ();

  preferences->sound_cards_property_bind_id = 0UL;

  xfconf_g_property_bind (preferences->channel, "/window-width", G_TYPE_INT, G_OBJECT (preferences), "window-width");
  xfconf_g_property_bind (preferences->channel, "/window-height", G_TYPE_INT, G_OBJECT (preferences), "window-height");
  xfconf_g_property_bind (preferences->channel, "/sound-card", G_TYPE_STRING, G_OBJECT (preferences), "sound-card");
}



static void
xfce_mixer_preferences_finalize (GObject *object)
{
  XfceMixerPreferences *preferences = XFCE_MIXER_PREFERENCES (object);

  if (preferences->sound_card != NULL)
    {
      g_free (preferences->sound_card);
      preferences->sound_card = NULL;
    }

  if (preferences->controls != NULL)
    {
      xfconf_array_free (preferences->controls);
      preferences->controls = NULL;
    }

  (*G_OBJECT_CLASS (xfce_mixer_preferences_parent_class)->finalize) (object);
}



static void 
xfce_mixer_preferences_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  XfceMixerPreferences *preferences = XFCE_MIXER_PREFERENCES (object);

  switch (prop_id)
    {
    case PROP_WINDOW_WIDTH:
      g_value_set_int (value, preferences->window_width);
      break;
    case PROP_WINDOW_HEIGHT:
      g_value_set_int (value, preferences->window_height);
      break;
    case PROP_SOUND_CARD:
      g_value_set_string (value, preferences->sound_card);
      break;
    case PROP_CONTROLS:
      g_value_set_boxed (value, preferences->controls);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void 
xfce_mixer_preferences_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  XfceMixerPreferences *preferences = XFCE_MIXER_PREFERENCES (object);
  gchar                *sound_cards_property_name;
  GPtrArray            *controls;
  guint                 i;
  GValue               *element;
  GValue               *control;

  switch (prop_id)
    {
    case PROP_WINDOW_WIDTH:
      preferences->window_width = g_value_get_int (value);
      break;
    case PROP_WINDOW_HEIGHT:
      preferences->window_height = g_value_get_int (value);
      break;
    case PROP_SOUND_CARD:
      /* Freeze "notify" signals since the "controls" property is also manipulated */
      g_object_freeze_notify (object);

      g_free (preferences->sound_card);
      preferences->sound_card = g_value_dup_string (value);

      /* Remove the previous binding */
      if (preferences->sound_cards_property_bind_id > 0)
        {
          xfconf_g_property_unbind (preferences->sound_cards_property_bind_id);
          preferences->sound_cards_property_bind_id = 0UL;
        }

      /* Make sure the "controls" property is reset */
      g_object_set (object, "controls", NULL, NULL);

      if (preferences->sound_card != NULL)
        {
          /* Bind to the property corresponding to the new sound card */
          sound_cards_property_name = g_strdup_printf ("/sound-cards/%s", preferences->sound_card);
          preferences->sound_cards_property_bind_id = xfconf_g_property_bind (preferences->channel, sound_cards_property_name, G_TYPE_PTR_ARRAY, G_OBJECT (preferences), "controls");
          g_free (sound_cards_property_name);
        }

      g_object_thaw_notify (object);
      break;
    case PROP_CONTROLS:
      if (preferences->controls != NULL)
        xfconf_array_free (preferences->controls);

      controls = g_value_get_boxed (value);
      if (controls != NULL)
        {
          preferences->controls = g_ptr_array_sized_new (controls->len);
          for (i = 0; i < controls->len; ++i)
            {
              element = (GValue *) g_ptr_array_index (controls, i);

              /* Filter out invalid value types */
              if (G_VALUE_HOLDS_STRING (element))
                {
                  control = g_value_init (g_new0 (GValue, 1), G_TYPE_STRING);
                  g_value_copy (element, control);
                  g_ptr_array_add (preferences->controls, control);
                }
            }
        }
      else
        preferences->controls = xfce_mixer_preferences_get_default_tracks (preferences);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



XfceMixerPreferences*
xfce_mixer_preferences_get (void)
{
  static XfceMixerPreferences *preferences = NULL;

  if (G_UNLIKELY (preferences == NULL))
    {
      preferences = g_object_new (TYPE_XFCE_MIXER_PREFERENCES, NULL);
      g_object_add_weak_pointer (G_OBJECT (preferences), (gpointer) &preferences);
    }
  else
    g_object_ref (G_OBJECT (preferences));

  return preferences;
}



void
xfce_mixer_preferences_set_controls (XfceMixerPreferences *preferences,
                                     GPtrArray            *controls)
{
  g_return_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences));
  g_return_if_fail (controls != NULL);

  g_object_set (G_OBJECT (preferences), "controls", controls, NULL);
}



GPtrArray *
xfce_mixer_preferences_get_controls (XfceMixerPreferences *preferences)
{
  GPtrArray *controls;

  g_return_val_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences), NULL);

  g_object_get (G_OBJECT (preferences), "controls", &controls, NULL);

  return controls;
}



gboolean
xfce_mixer_preferences_get_control_visible (XfceMixerPreferences *preferences,
                                            const gchar          *track_label)
{
  gboolean       visible = FALSE;
  guint          i;

  g_return_val_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (preferences->controls != NULL, FALSE);

  for (i = 0; i < preferences->controls->len; ++i)
    {
      if (xfce_mixer_utf8_cmp (g_value_get_string (g_ptr_array_index (preferences->controls, i)), track_label) == 0)
        {
          visible = TRUE;
          break;
        }
    }

  return visible;
}



static GPtrArray *
xfce_mixer_preferences_get_default_tracks (XfceMixerPreferences *preferences)
{
  GList       *track_list;
  GList       *iter;
  GPtrArray   *tracks;
  GstElement  *card;
  const gchar *track_label;
  GValue      *value;

  tracks = g_ptr_array_new ();

  if (preferences->sound_card != NULL)
    {
      card = xfce_mixer_get_card (preferences->sound_card);

      if (GST_IS_MIXER (card))
        {
          track_list = xfce_mixer_get_default_track_list (card);
          for (iter = track_list; iter != NULL; iter = g_list_next (iter))
            {
              value = g_value_init (g_new0 (GValue, 1), G_TYPE_STRING);
              track_label = xfce_mixer_get_track_label (GST_MIXER_TRACK (iter->data));
              g_value_set_string (value, track_label);
              g_ptr_array_add (tracks, value);
            }
        }
    }

  return tracks;
}

