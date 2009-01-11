#ifndef __MIXER_VCS_H
#define __MIXER_VCS_H

/* call this at the beginning of your program. */
int register_vcs ();

/* Optionally register a event callback (per device) with
     vc_set_volume_callback(callback, user_data);
   call vc_handle_events() periodically.
 */

#include "vc.h"

#endif
