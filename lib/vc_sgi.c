/*
 * Copyright (c) 2005 Daichi Kawahata <daichi@xfce.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_SGI

#define USE_THAT 1

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <audio.h>
#include <sys/hdsp.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#define VC_PLUGIN
#include "vc.h"

#define MAX_CHANNELS 10
#define MIXER_BASE AL_DEFAULT_OUTPUT
#define STRING_SIZE 32

static int mixer_resource = -1;
static int fd = -1;

static int cnt_interfaces = 0;
static ALvalue interfaces[16];

static void vc_close_device()
{
	if (fd != -1) {
		close (fd);
		fd = -1;
	}
}

static int vc_get_device_resource_by_name(gchar const* name);
static gchar* vc_get_resource_name(int resource);

static int vc_reinit_device(void)
{
    gchar* mixer_device_name;
    int i;
    
    vc_close_device ();

    mixer_device_name = vc_get_resource_name (mixer_resource);
    
    if (alIsSubtype(AL_DEVICE_TYPE, mixer_resource)) {
       /*
        * We call alQueryValues to get the set of interfaces
        * on this device
        */
        cnt_interfaces = alQueryValues(mixer_resource, AL_INTERFACE, interfaces, sizeof(interfaces)/sizeof(interfaces[0]), 0, 0); /* why 16? */
        if (cnt_interfaces >= 0) {
          /*vc_get_resource_name (interfaces[i].i);*/
        }
        else {
            g_warning ("failed to get list of interfaces of device %s: %s\n", mixer_device_name, alGetErrorString(oserror()));
        }
    }

    g_free (mixer_device_name);
    return 0;
}

static void vc_set_device(char const *name)
{
    if (name == NULL) {
      name = "default";
    }
    
    mixer_resource = vc_get_device_resource_by_name(name);
    vc_reinit_device ();
}

static int vc_get_volume(char const *which)
{
     ALpv parameters[4];
     ALpv y;
     ALparamInfo ainfo;
     ALfixed gains[MAX_CHANNELS];
     int cnt_channels;
     int vol_p;
     
     int i;

     double median;
     double max;
     int cnt_on_channels;
     
     /*
      * Now get information about the supported values for
      * gain.
      */
     alGetParamInfo(mixer_resource, AL_GAIN, &ainfo);

     /*min = alFixedToDouble(ainfo.min.ll);*/
     max = alFixedToDouble(ainfo.max.ll);

     /*printf("min: %s dB; max: %lf dB; min delta: %lf dB\n\n",
        min,alFixedToDouble(ainfo.max.ll), alFixedToDouble(ainfo.minDelta.ll));*/
        
     /*
      * Now get the current value of gain
      */

     parameters[0].param = AL_GAIN;
     parameters[0].value.ptr = gains;
     parameters[0].sizeIn = MAX_CHANNELS;   /* we've provided an 8-channel vector */
     parameters[1].param = AL_CHANNELS;
     /* ^--- ??? */
     
     if (alGetParams(mixer_resource, parameters, 2) < 0) {
        g_warning ("vc_sgi.c: vc_get_volume: alGetParams failed: %s\n", alGetErrorString(oserror()));
        return 0;
     }

     if (parameters[0].sizeOut < 0) {
        g_warning ("vc_sgi.c: vc_get_volume: AL_GAIN was an unrecognized parameter");
        return 0;
     }
     
     cnt_channels = parameters[0].sizeOut;
     /*printf("(%d channels) gain:\n       ", cnt_channels);*/
            
     median = 0.0;
     cnt_on_channels = 0;
     for (i = 0; i < cnt_channels; i++) {
        /* FIXME is that guaranteed to be fixed point? */
        
        if (gains[i] == AL_NEG_INFINITY) { /* off */
          /*printf("%d: off\n", i);*/
        } else {
          ++cnt_on_channels;
          
          /*printf("%d: %lf dB\n", i, alFixedToDouble(gains[i]));*/
          median = median + alFixedToDouble(gains[i]);
        }
     }
     if (cnt_on_channels > 0) {
        median = median / cnt_on_channels;
     } else {
        median = 0.0;
     }
	    
     if (max > 0.0) {
        vol_p = (int)(100.0 * median / max);
        if (vol_p < 0) {
          vol_p = 0;
        }
        
        if (vol_p > 100) {
          vol_p = 100;
        }
        
        return vol_p;
     }

     return 0;
}

static void vc_set_volume(char const *which, int vol_p)
{
     ALpv parameters[2];
     ALfixed raw_value;
     ALfixed gains[MAX_CHANNELS];
     int cnt_channels;
     int i;
     ALparamInfo ainfo;

     /*
      * Now get information about the supported values for
      * gain.
      */
     alGetParamInfo(mixer_resource, AL_GAIN, &ainfo);

     if (vol_p == 0) {
       if (ainfo.specialVals & AL_NEG_INFINITY_BIT) {
         raw_value = AL_NEG_INFINITY;
       } else {
         raw_value = ainfo.min.ll;
       }
     } else {
       raw_value = ainfo.max.ll * vol_p / 100;
     }

     /* now get the number of channels in an extremely inefficient way */
     
     parameters[0].param = AL_GAIN;
     parameters[0].value.ptr = gains;
     parameters[0].sizeIn = MAX_CHANNELS;   /* we've provided an 8-channel vector */
     parameters[1].param = AL_CHANNELS;
     /* ^--- ??? */
     
     if (alGetParams(mixer_resource, parameters, 2) < 0) {
        g_warning ("vc_sgi.c: vc_set_volume: alGetParams failed: %s\n", alGetErrorString(oserror()));
        return;
     }

     if (parameters[0].sizeOut < 0) {
        g_warning ("vc_sgi.c: vc_set_volume: AL_GAIN was an unrecognized parameter");
        return;
     }
     
     cnt_channels = parameters[0].sizeOut;
     
     /* and set the new value for all channels */
     
     for (i = 0; i < cnt_channels; i++) {
       gains[i] = raw_value;
     }
     
     parameters[0].param = AL_GAIN;
     parameters[0].value.ptr = gains;
     parameters[0].sizeIn = MAX_CHANNELS;   /* we've provided an 8-channel vector */
     parameters[1].param = AL_CHANNELS;
     /* ^--- ??? */
     
     if (alSetParams(mixer_resource, parameters, 2) < 0) {
        g_warning ("vc_sgi.c: vc_set_volume: alSetParams failed: %s\n", alGetErrorString(oserror()));
        return;
     }
}

static volcontrol_t *create_volcontrol_slider(char const *name)
{
    volcontrol_t *vc;
    vc = g_new0 (volcontrol_t, 1); 
    vc->name = g_strdup (name);
    vc->type = CT_SLIDER;
    return vc;
}

/* returns list of volcontrol_t */
static GList *vc_get_control_list()
{
    GList *controls;
    gchar* name;
    int i;

    controls = NULL;
    
    for (i = 0; i < cnt_interfaces; i++) {
      name = vc_get_resource_name (interfaces[i].i);
      if (name != NULL) {
        controls = g_list_append (controls, create_volcontrol_slider (name));
        g_free (name);
      }
    }
    
    /*controls = g_list_append (controls, create_volcontrol_slider ("Output Volume"));
    controls = g_list_append (controls, create_volcontrol_slider ("Record Volume"));
    controls = g_list_append (controls, create_volcontrol_slider ("Monitor Volume"));
    */

    return controls;
}

static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* unsupported */
}

static gchar* vc_get_resource_name(int resource)
{
    ALpv parameters[1];
    char device_name[STRING_SIZE];
    /*char device_label[STRING_SIZE];*/
    
    parameters[0].param = AL_NAME;
    parameters[0].value.ptr = device_name;
    parameters[0].sizeIn = STRING_SIZE;      /* pass in max size of string */

    if (alGetParams(resource, parameters, sizeof(parameters) / sizeof(parameters[0])) < 0) { /* get the resource name & label */
      g_warning ("vc_sgi: failed to get parameter NAME of interface %d", resource);
      return NULL;
    }
    
/*        parameters[1].param = AL_LABEL;
        parameters[1].value.ptr = device_label;
        parameters[1].sizeIn = STRING_SIZE;      /* pass in max size of string 
*/

    return g_strdup (device_name);

}

static int vc_get_device_resource_by_name(gchar const* name)
{
    /* slow */
  
    int i;
    int resource;
    int cnt_devices;
    ALvalue devices[32];
    GList* res;
    gchar* xname;
    
    res = NULL;
    cnt_devices = alQueryValues(AL_SYSTEM, AL_DEVICES, devices, 16, 0, 0); /* why 16? */

    for (i = 0; i < cnt_devices; i++) {
        /*
         * Get the text labels associated with the source and
         * destination resources, so our printout is more readable.
         */
        resource = devices[i].i;
        
        xname = vc_get_resource_name (resource);
        if (xname != NULL) {
          if (name == "default" || g_str_equal (xname, name)) { /* first as default for now */
            g_free (xname);
            return resource;
          }
          
          g_free (xname);
        }
    }
    
    g_warning ("vc_sgi: could not find device \"%s\"", name);
    return -1;
}

static GList* vc_get_device_list_irix(ALvalue devices[], int cnt_devices)
{
    ALpv parameters[2];
    char device_name[STRING_SIZE];
    char device_label[STRING_SIZE];
    int i;
    int resource;
    GList* res;
    
    res = NULL;

    for (i = 0; i < cnt_devices; i++) {

        /*
         * Get the text labels associated with the source and
         * destination resources, so our printout is more readable.
         */
        parameters[0].param = AL_LABEL;
        parameters[0].value.ptr = device_label;
        parameters[0].sizeIn = STRING_SIZE;      /* pass in max size of string */
        parameters[1].param = AL_NAME;
        parameters[1].value.ptr = device_name;
        parameters[1].sizeIn = STRING_SIZE;      /* pass in max size of string */
        
        resource = devices[i].i;
        if (alGetParams(resource, parameters, sizeof(parameters) / sizeof(parameters[0])) < 0) { /* get the resource name & label */
          g_warning ("vc_sgi: failed to get parameters LABEL and NAME of interface %d", resource);
          continue;
        }
        
        res = g_list_append (res, g_strdup (device_name)); /* add name to list */
    }
    
    return res;
}

static GList *vc_get_device_list()
{
    int fd, i;
    int cnt_devices;
    ALvalue devices[32];
    GList* res;
    res = NULL;

    /*
     * We call alQueryValues to get the set of devices
     * on the system.
     */
    cnt_devices = alQueryValues(AL_SYSTEM, AL_DEVICES, devices, 16, 0, 0); /* why 16? */
    
    if (cnt_devices >= 0) {
        res = vc_get_device_list_irix(devices, cnt_devices);
    } else {
        g_warning ("vc_sgi: failed to get list of devices: %d\n", oserror());
    }
    return res;
}

static void vc_set_select(char const *which, gchar const *v)
{
}

static gchar *vc_get_select(char const *which)
{
	return NULL;
}

static void vc_set_switch(char const *which, gboolean v)
{
}

static gboolean vc_get_switch(char const *which)
{
	return FALSE;
}

static char const *vc_get_device()
{
	return NULL;
}

static void vc_handle_events()
{
}

static int init(void)
{
	vc_set_device("default");
	vc_reinit_device();
	return USE_THAT;
}

REGISTER_VC_PLUGIN(sgi);

#endif /* !USE_SGI */
