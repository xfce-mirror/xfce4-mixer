#include <gtk/gtk.h>
#include "xfce-mixer-control.h"
#include "xfce-mixer-slider.h"
#include "xfce-mixer-slider-tiny.h"
#include "xfce-mixer-switch.h"
#include "xfce-mixer-select.h"
#include "xfce-mixer-profile.h"
#include "xfce-mixer-control-factory.h"
#include "xfce-mixer-cache-vc.h"
#include "xfce-mixer-control-vc.h"
#include "xfce-mixer-mcs-client.h"
#include "vc.h"

extern XfceMixerMcsClient *mcsc;

XfceMixerControl *xfce_mixer_control_factory_new_from_profile_item(
	t_mixer_profile_item *item,
	t_mixer_control_factory_kind k
)
{
	XfceMixerControl *c;
	gchar ty;
	gchar *new_loc;
	gchar *uc;
	gchar *uct;
	gboolean b;
	
	new_loc = NULL;
	
	c = NULL;
	if (item && item->vcname) {
		if (!xfce_mixer_cache_vc_valid (item->vcname)) {
			return c;
		}
		b = TRUE;

		if (mcsc)
			uc = xfce_mixer_mcs_client_get_useful_controls (mcsc, 
				vc_get_device ());
			
		if (mcsc && uc) {
			uct = g_strdup_printf ("@%s@", item->vcname);
			if (uct) {
				b = g_strrstr (uc, uct) != NULL;
				/*printf ("%s %s %d", uc , uct, (int)b );*/
			}
			g_free (uc);
		}
		
		if (!b)
			return c;
			
		ty = xfce_mixer_cache_vc_get_type (item->vcname); 
		
		if (ty != 'S' && k == K_TINY)
			return c;
			
		if (ty == 'D')
			return c;
	
		if (k == K_TINY) {
			c = XFCE_MIXER_CONTROL (xfce_mixer_slider_tiny_new ());
		} else {
			switch (ty) {
			case 'S':
				c = xfce_mixer_slider_new (); 
				if (c) new_loc = g_strdup ("sliders");
				break;
			case 'O': 
				c = xfce_mixer_switch_new (); 
				if (c) new_loc = g_strdup ("switches");
				break;
			case 'C':
				c = xfce_mixer_select_new (); 
				if (c) new_loc = g_strdup ("selects");
				break;
			}
		}
		if (!c)
			return c;

		if (new_loc) {
			if (item->location && g_str_equal (item->location, "default")) {
				g_free (item->location);
				item->location = new_loc;
			} else {
				g_free (new_loc);
			}
			new_loc = NULL;
		}
		
		g_object_set (G_OBJECT (c), 
			"location", item->location, 
			"orderno", item->orderno,
			"vcname", item->vcname,
			NULL
		);
		
		/*g_signal_connect (G_OBJECT (c), "::notify::value", );*/
		
		xfce_mixer_control_vc_feed_value (c);
		xfce_mixer_control_vc_attach (c);

	}
	
	return c;
}


