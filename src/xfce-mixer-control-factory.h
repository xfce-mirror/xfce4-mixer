#ifndef __XFCE_MIXER_CONTROL_FACTORY_H
#define __XFCE_MIXER_CONTROL_FACTORY_H

#include "xfce-mixer-profile.h"
#include "xfce-mixer-control.h"

typedef enum {
	K_NORMAL,
	K_TINY
} t_mixer_control_factory_kind;

XfceMixerControl *xfce_mixer_control_factory_new_from_profile_item(t_mixer_profile_item *item, t_mixer_control_factory_kind k);

#endif
