#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_ALSA
extern int register_alsa ();
#endif
        
#ifdef USE_OSS
extern int register_oss ();
#endif

#ifdef USE_SUN
extern int register_sun ();
#endif

int register_vcs ()
{
#ifdef USE_ALSA
	return register_alsa ();
#endif
        
#ifdef USE_OSS
	return register_oss ();
#endif

#ifdef USE_SUN
	return register_sun ();
#endif

	return -1;
}