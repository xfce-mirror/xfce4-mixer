/* connector between worlds (vc) and (Xfce:Mixer:Control). cache_vc is the second part. */
#include <stdio.h>
#include <stdlib.h>
#include "xfce-mixer-control-vc.h"
#include "xfce-mixer-switch.h"
#include "xfce-mixer-select.h"
#include "vc.h"

static
void value_changed_cb(XfceMixerControl *control, gpointer whatsthat, gpointer user_data)
{
	gint v;
	gboolean b;

	if (!control || !control->vcname)
		return;
		
	if (XFCE_IS_MIXER_SWITCH (control)) {
		b = control->value && (control->value[0] == '1');
		vc_set_switch (control->vcname, b);
		return;
	}

	if (XFCE_IS_MIXER_SELECT (control)) {
		vc_set_select (control->vcname, control->value);
		return;
	}
	
	/*if (XFCE_IS_MIXER_SLIDER (control)) {*/
		if (control->value && (sscanf (control->value, "%d", &v) > 0)) {
			if (v != vc_get_volume (control->vcname))
				vc_set_volume (control->vcname, v);
		}
	/*}*/
}

void xfce_mixer_control_vc_attach (XfceMixerControl *c)
{
	g_signal_connect (G_OBJECT (c), "notify::value", G_CALLBACK (value_changed_cb), NULL);
}
 
void xfce_mixer_control_vc_feed_value(XfceMixerControl *c)
{
	gint v;
	gboolean b;
	gchar *tmp;
	if (!c || !c->vcname)
		return;

	if (XFCE_IS_MIXER_SWITCH (c)) {
		b = vc_get_switch (c->vcname);
		tmp = g_strdup (b ? "1" : "0");
	} else if (XFCE_IS_MIXER_SELECT (c)) {
		tmp = vc_get_select (c->vcname);
	} else {
		v = vc_get_volume (c->vcname);
		tmp = g_strdup_printf ("%d", v);
	}
	g_object_set (G_OBJECT (c), "value", tmp, NULL);
	g_free (tmp);
}

