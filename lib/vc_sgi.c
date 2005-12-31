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

#define MAX_MIXERS 10
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
    mixer_resource = vc_get_device_resource_by_name(name);
    vc_reinit_device ();
}

static int vc_get_volume(char const *which)
{
     int rv;
     ALpv x[4];
     ALpv y;
     ALparamInfo ainfo;
     ALfixed gain[8];
     int i;
     char min[30];


     return 0;
     
     /*
      * Now get information about the supported values for
      * gain.
      */
     alGetParamInfo(rv, AL_GAIN, &ainfo);

     /*
      * One of the "special" values not described in the normal
      * min->max range is negative infinity. See if this value
      * is supported.
      */
     if (ainfo.specialVals & AL_NEG_INFINITY_BIT) {
         sprintf(min, "-inf, %lf", alFixedToDouble(ainfo.min.ll));
     }
     else {
         sprintf(min, "%lf", alFixedToDouble(ainfo.min.ll));
     }

     /*
      * Print out the gain range.
      */
     printf("min: %s dB; max: %lf dB; min delta: %lf dB\n\n",
        min,alFixedToDouble(ainfo.max.ll), alFixedToDouble(ainfo.minDelta.ll));
     /*
      * Now get the current value of gain
      */

     x[0].param = AL_GAIN;
     x[0].value.ptr = gain;
     x[0].sizeIn = 8;                   /* we've provided an 8-channel
                                           vector */
     x[1].param = AL_CHANNELS;
     if (alGetParams(rv, x, 2) < 0) {
        printf("getparams failed: %d\n", oserror());
     }
     else {
        if (x[0].sizeOut < 0) {
            printf("AL_GAIN was an unrecognized parameter\n");
        }
        else {
            printf("(%d channels) gain:\n       ", x[0].sizeOut);
            for (i = 0; i < x[0].sizeOut; i++) {
                printf("%lf dB ", alFixedToDouble(gain[i]));
            }
            printf("\n");
        }
     }

	return 0;
}

static void vc_set_volume(char const *which, int vol_p)
{
     int fd;
     int rv;
     double rate;
     ALpv x[2];
     
     return;

     /*
      * Now set the nominal rate to the given number, and
      * set AL_MASTER_CLOCK to be AL_CRYSTAL_MCLK_TYPE.
      */
     x[0].param = AL_RATE;
     x[0].value.ll = alDoubleToFixed(rate);
     x[1].param = AL_MASTER_CLOCK;
     x[1].value.i = AL_CRYSTAL_MCLK_TYPE;

     if (alSetParams(rv,x, 2)<0) {
        printf("setparams failed: %s\n",alGetErrorString(oserror()));
     }
     if (x[0].sizeOut < 0) {
        /*
         * Not all devices will allow setting of AL_RATE (for example, digital 
         * inputs run only at the frequency of the external device).  Check
         * to see if the rate was accepted.
         */
        printf("AL_RATE was not accepted on the given resource\n");
     }

     if (alGetParams(rv,x, 1)<0) {
        printf("getparams failed: %s\n",alGetErrorString(oserror()));
     }
     printf("rate is now %lf\n",alFixedToDouble(x[0].value.ll));

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
