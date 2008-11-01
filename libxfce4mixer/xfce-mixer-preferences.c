/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
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
  N_PROPERTIES,
};



static void   xfce_mixer_preferences_class_init        (XfceMixerPreferencesClass *klass);
static void   xfce_mixer_preferences_init              (XfceMixerPreferences      *preferences);
static void   xfce_mixer_preferences_finalize          (GObject                   *object);
static void   xfce_mixer_preferences_get_property      (GObject                   *object,
                                                        guint                      prop_id,
                                                        GValue                    *value,
                                                        GParamSpec                *pspec);
static void   xfce_mixer_preferences_set_property      (GObject                   *object,
                                                        guint                      prop_id,
                                                        const GValue              *value,
                                                        GParamSpec                *pspec);
static void   xfce_mixer_preferences_load              (XfceMixerPreferences      *preferences);
static void   xfce_mixer_preferences_store             (XfceMixerPreferences      *preferences);



struct _XfceMixerPreferencesClass
{
  GObjectClass __parent__;
};

struct _XfceMixerPreferences
{
  GObject        __parent__;

  XfconfChannel *channel;
  GHashTable    *controls;

  GValue         values[N_PROPERTIES];
};



static GObjectClass *xfce_mixer_preferences_parent_class = NULL;



GType
xfce_mixer_preferences_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerPreferencesClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_preferences_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerPreferences),
          0,
          (GInstanceInitFunc) xfce_mixer_preferences_init,
          NULL,
        };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMixerPreferences", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_preferences_class_init (XfceMixerPreferencesClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_preferences_parent_class = g_type_class_peek_parent (klass);

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
}



static void
xfce_mixer_preferences_init (XfceMixerPreferences *preferences)
{
    preferences->channel = xfconf_channel_get ("xfce4-mixer");
    preferences->controls = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_strfreev);

    xfconf_g_property_bind (preferences->channel, "/window-width", G_TYPE_INT, G_OBJECT (preferences), "window-width");
    xfconf_g_property_bind (preferences->channel, "/window-height", G_TYPE_INT, G_OBJECT (preferences), "window-height");
    xfconf_g_property_bind (preferences->channel, "/sound-card", G_TYPE_STRING, G_OBJECT (preferences), "sound-card");

    xfce_mixer_preferences_load (preferences);
}



static void
xfce_mixer_preferences_finalize (GObject *object)
{
  XfceMixerPreferences *preferences = XFCE_MIXER_PREFERENCES (object);

  g_hash_table_unref (preferences->controls);

  (*G_OBJECT_CLASS (xfce_mixer_preferences_parent_class)->finalize) (object);
}



static void 
xfce_mixer_preferences_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  XfceMixerPreferences *preferences = XFCE_MIXER_PREFERENCES (object);
  GValue               *src;

  src = preferences->values + prop_id;

  if (G_LIKELY (G_IS_VALUE (src)))
    g_value_copy (src, value);
  else
    g_param_value_set_default (pspec, value);
}



static void 
xfce_mixer_preferences_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  XfceMixerPreferences *preferences = XFCE_MIXER_PREFERENCES (object);
  GValue               *dest;

  dest = preferences->values + prop_id;

  if (G_UNLIKELY (!G_IS_VALUE (dest)))
    {
      g_value_init (dest, pspec->value_type);
      g_param_value_set_default (pspec, dest);
    }

  if (G_LIKELY (g_param_values_cmp (pspec, value, dest) != 0))
    g_value_copy (value, dest);
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



static void
xfce_mixer_preferences_load_controls (const gchar          *property_name,
                                      const GValue         *value,
                                      XfceMixerPreferences *preferences)
{
  gchar **controls;
  gchar  *card_name;

  g_return_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences));
  g_return_if_fail (XFCONF_IS_CHANNEL (preferences->channel));

  if (G_UNLIKELY ((card_name = g_strrstr (property_name, "/")) == NULL))
    return;

  /* Remove the leading slash */
  card_name = card_name + 1;

  /* Read controls for this card */
  controls = xfconf_channel_get_string_list (preferences->channel, property_name);

  if (G_LIKELY (controls != NULL))
    {
      /* Store controls in the hash table */
      g_hash_table_insert (preferences->controls, g_strdup (card_name), controls);
    }
}



static void 
xfce_mixer_preferences_load (XfceMixerPreferences *preferences)
{
  GHashTable *properties;

  g_return_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences));
  g_return_if_fail (XFCONF_IS_CHANNEL (preferences->channel));

  properties = xfconf_channel_get_properties (preferences->channel, "/sound-cards");

  if (G_LIKELY (properties != NULL))
    g_hash_table_foreach (properties, (GHFunc) xfce_mixer_preferences_load_controls, preferences);
}



static void
xfce_mixer_preferences_store_controls (const gchar          *card_name,
                                       gchar * const        *controls,
                                       XfceMixerPreferences *preferences)
{
  gchar *property_name;

  g_return_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences));
  g_return_if_fail (XFCONF_IS_CHANNEL (preferences->channel));

  property_name = g_strdup_printf ("/sound-cards/%s", card_name);

  if (G_UNLIKELY (controls != NULL))
    xfconf_channel_set_string_list (preferences->channel, property_name, (const gchar * const *) controls);
  else
    xfconf_channel_remove_property (preferences->channel, property_name);

  g_free (property_name);
}



static void
xfce_mixer_preferences_update_controls (const gchar          *property_name,
                                        const GValue         *value,
                                        XfceMixerPreferences *preferences)
{
  g_return_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences));
  g_return_if_fail (XFCONF_IS_CHANNEL (preferences->channel));

  xfconf_channel_remove_property (preferences->channel, property_name);
}



static void 
xfce_mixer_preferences_store (XfceMixerPreferences *preferences)
{
  GHashTable *properties;

  g_return_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences));
  g_return_if_fail (XFCONF_IS_CHANNEL (preferences->channel));

  properties = xfconf_channel_get_properties (preferences->channel, "/sound-cards");

  if (G_LIKELY (properties != NULL))
    g_hash_table_foreach (properties, (GHFunc) xfce_mixer_preferences_update_controls, preferences);

  g_hash_table_foreach (preferences->controls, (GHFunc) xfce_mixer_preferences_store_controls, preferences);
}



gchar * const *
xfce_mixer_preferences_get_visible_controls (XfceMixerPreferences *preferences,
                                             GstElement           *card)
{
  const gchar *card_name;

  g_return_val_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences), NULL);
  g_return_val_if_fail (GST_IS_MIXER (card), NULL);

  card_name = xfce_mixer_get_card_internal_name (card);
  return (gchar * const *) g_hash_table_lookup (preferences->controls, card_name);
}



void
xfce_mixer_preferences_set_visible_controls (XfceMixerPreferences *preferences,
                                             GstElement           *card,
                                             gchar * const        *controls)
{
  const gchar *card_name;
  g_return_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences));
  g_return_if_fail (GST_IS_MIXER (card));

  card_name = xfce_mixer_get_card_internal_name (card);
  g_hash_table_insert (preferences->controls, g_strdup (card_name), g_strdupv ((gchar **) controls));
  xfce_mixer_preferences_store (preferences);
}



gboolean
xfce_mixer_preferences_get_control_visible (XfceMixerPreferences *preferences,
                                            GstElement           *card,
                                            GstMixerTrack        *track)
{
  gchar * const *controls;
  const gchar   *card_name;
  gboolean       visible = FALSE;
  gint           i;

  g_return_val_if_fail (IS_XFCE_MIXER_PREFERENCES (preferences), FALSE);
  g_return_val_if_fail (GST_IS_MIXER (card), FALSE);
  g_return_val_if_fail (GST_IS_MIXER_TRACK (track), FALSE);

  card_name = xfce_mixer_get_card_internal_name (card);
  controls = g_hash_table_lookup (preferences->controls, card_name);

  if (G_LIKELY (controls != NULL))
    {
      for (i = 0; controls != NULL && controls[i] != NULL; ++i)
        if (g_utf8_collate (controls[i], track->label) == 0)
          {
            visible = TRUE;
            break;
          }
    }

  return visible;
}
