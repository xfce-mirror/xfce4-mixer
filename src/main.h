#ifndef __MAIN_H
#define __MAIN_H
#include "xfce-mixer-profiles.h"

#define CHANNEL "sound"

extern gchar *device;
extern XfceMixerProfiles *profiles;

gboolean delayed_refresh_cb(gpointer p);

#endif
