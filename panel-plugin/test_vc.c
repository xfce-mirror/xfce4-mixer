
/* compile:

gcc -o test_vc test_vc.c vc.c $(pkg-config --cflags --libs gtk+-2.0) vc_oss.c -DUSE_OSS $(pkg-config --cflags --libs libxfce4util-1.0) -DHAVE_SYS_SOUNDCARD_H -DHAVE_UNISTD_H -DHAVE_SYS_TYPES_H -DHAVE_FCNTL_H

*/

#include <stdio.h>
#include "vc.h"

extern int register_oss(void);

int main()
{
	register_oss ();

	select_vc_direct (*first_vc ());

	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	
	vc_set_device ("/dev/mixer1");

	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_get_volume (NULL);
	vc_set_volume (NULL, 50);
}
