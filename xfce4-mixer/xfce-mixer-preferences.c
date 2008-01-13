/* $Id$ */
/* vim:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your preferences) any later version.
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

#include <glib-object.h>

#include <libxfce4util/libxfce4util.h>

#include "xfce-mixer-preferences.h"



enum
{
  PROP_0,
  PROP_LAST_WINDOW_WIDTH,
  PROP_LAST_WINDOW_HEIGHT,
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
static gchar *xfce_mixer_preferences_get_mixer_rc_group (XfceMixerPreferences      *preferences,
                                                        const gchar               *mixer);



struct _XfceMixerPreferencesClass
{
  GObjectClass __parent__;
};

struct _XfceMixerPreferences
{
  GObject __parent__;

  GValue  values[N_PROPERTIES];
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
                                   PROP_LAST_WINDOW_WIDTH,
                                   g_param_spec_int ("last-window-width",
                                                     "last-window-width",
                                                     "last-window-width",
                                                     1, G_MAXINT, 600,
                                                     G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class, 
                                   PROP_LAST_WINDOW_HEIGHT,
                                   g_param_spec_int ("last-window-height",
                                                     "last-window-height",
                                                     "last-window-height",
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
}



static void
xfce_mixer_preferences_finalize (GObject *object)
{
  XfceMixerPreferences *preferences = XFCE_MIXER_PREFERENCES (object);

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
    {
      g_value_copy (value, dest);
      xfce_mixer_preferences_store (preferences);
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
      xfce_mixer_preferences_load (preferences);
    }
  else
    g_object_ref (G_OBJECT (preferences));

  return preferences;
}



static void 
xfce_mixer_preferences_load (XfceMixerPreferences *preferences)
{
  const gchar *value;
  GParamSpec **specs;
  GParamSpec  *spec;
  GValue       dest = { 0, };
  GValue       src = { 0, };
  XfceRc      *rc;
  guint        nspecs;
  guint        n;

  rc = xfce_mixer_preferences_get_rc (preferences);

  if (G_UNLIKELY (rc == NULL))
    {
      g_warning ("Failed to load the mixer preferences.");
      return;
    }

  g_object_freeze_notify (G_OBJECT (preferences));

  xfce_rc_set_group (rc, "Configuration");

  specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (preferences), &nspecs);

  for (n = 0; n < nspecs; ++n)
    {
      spec = specs[n];

      value = xfce_rc_read_entry (rc, spec->name, NULL);

      if (G_UNLIKELY (value == NULL))
        continue;

      g_value_init (&src, G_TYPE_STRING);
      g_value_set_static_string (&src, value);

      if (spec->value_type == G_TYPE_STRING)
        g_object_set_property (G_OBJECT (preferences), spec->name, &src);
      else if (g_value_type_transformable (G_TYPE_STRING, spec->value_type))
        {
          g_value_init (&dest, spec->value_type);
          if (g_value_transform (&src, &dest))
            g_object_set_property (G_OBJECT (preferences), spec->name, &dest);
          g_value_unset (&dest);
        }
      else
        g_warning ("Failed to load property \"%s\"", spec->name);

      g_value_unset (&src);
    }

  xfce_rc_close (rc);

  g_object_thaw_notify (G_OBJECT (preferences));
}



void 
xfce_mixer_preferences_store (XfceMixerPreferences *preferences)
{
  const gchar *value;
  GParamSpec **specs;
  GParamSpec  *spec;
  GValue       dest = { 0, };
  GValue       src = { 0, };
  XfceRc      *rc;
  guint        nspecs;
  guint        n;

  rc = xfce_mixer_preferences_get_rc (preferences);

  if (G_UNLIKELY (rc == NULL))
    {
      g_warning ("Failed to save the mixer preferences");
      return;
    }

  xfce_rc_set_group (rc, "Configuration");

  specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (preferences), &nspecs);

  for (n = 0; n < nspecs; ++n)
    {
      spec = specs[n];

      g_value_init (&dest, G_TYPE_STRING);

      if (spec->value_type == G_TYPE_STRING)
        g_object_get_property (G_OBJECT (preferences), spec->name, &dest);
      else
        {
          g_value_init (&src, spec->value_type);
          g_object_get_property (G_OBJECT (preferences), spec->name, &src);
          g_value_transform (&src, &dest);
          g_value_unset (&src);
        }

      value = g_value_get_string (&dest);

      if (G_LIKELY (value != NULL))
        xfce_rc_write_entry (rc, spec->name, value);

      g_value_unset (&dest);
    }

  xfce_rc_close (rc);
}



XfceRc*
xfce_mixer_preferences_get_rc (XfceMixerPreferences *preferences)
{
  return xfce_rc_config_open (XFCE_RESOURCE_CONFIG, "xfce4/mixerrc", FALSE);
}



GList*
xfce_mixer_preferences_get_visible_controls (XfceMixerPreferences *preferences,
                                             const gchar          *mixer)
{
  XfceRc  *rc;
  GList   *list = NULL;
  gchar  **entries;
  gchar   *group;
  gint     i;

  rc = xfce_mixer_preferences_get_rc (preferences);

  if (G_UNLIKELY (rc == NULL))
    return NULL;

  group = xfce_mixer_preferences_get_mixer_rc_group (preferences, mixer);
  xfce_rc_set_group (rc, group);
  entries = xfce_rc_get_entries (rc, group);
  g_free (group);

  for (i = 0; entries != NULL && entries[i] != NULL; ++i)
    {
      if (xfce_rc_read_bool_entry (rc, entries[i], FALSE))
        list = g_list_prepend (list, g_strdup (entries[i]));
    }

  g_strfreev (entries);

  xfce_rc_close (rc);

  return list;
}



static gchar*
xfce_mixer_preferences_get_mixer_rc_group (XfceMixerPreferences *preferences,
                                           const gchar          *mixer)
{
  return g_strdup_printf ("Card %s", mixer);
}



void
xfce_mixer_preferences_set_control_visible (XfceMixerPreferences *preferences,
                                            const gchar          *mixer,
                                            const gchar          *control,
                                            gboolean              visible)
{
  XfceRc *rc;
  gchar  *group;

  rc = xfce_mixer_preferences_get_rc (preferences);

  if (G_UNLIKELY (rc == NULL))
    {
      g_warning ("Failed to save control visibility configuration");
      return;
    }

  group = xfce_mixer_preferences_get_mixer_rc_group (preferences, mixer);
  xfce_rc_set_group (rc, group);
  g_free (group);

  if (visible)
    xfce_rc_write_bool_entry (rc, control, TRUE);
  else
    xfce_rc_delete_entry (rc, control, FALSE);

  xfce_rc_close (rc);
}
