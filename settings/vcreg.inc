
extern int register_alsa();
extern int register_oss();

#ifdef USE_ALSA

#define VC_INIT \
	register_alsa ();

#else
#ifdef USE_OSS

#define VC_INIT \
	register_oss ();

#else

#define VC_INIT

#endif
#endif
