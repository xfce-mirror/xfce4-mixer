/* connector between worlds (vc) and (Xfce:Mixer:Control). cache_vc is the second part. */
#include <stdio.h>
#include <stdlib.h>
#include "xfce-mixer-control-vc.h"
#include "vc.h"

static
void value_changed_cb(XfceMixerControl *control, gpointer whatsthat, gpointer user_data)
{
	if (!control || !control->vcname)
		return;
		
	gint v;
	
	/*if (XFCE_IS_MIXER_SLIDER (control)) {*/
		if (control->value && (sscanf (control->value, "%d", &v) > 0)) {
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
	gchar *tmp;
	if (!c || !c->vcname)
		return;

	g_signal_handlers_block_by_func (G_OBJECT (c), value_changed_cb, NULL); 
		
	v = vc_get_volume (c->vcname);
	tmp = g_strdup_printf ("%d", v);
	g_object_set (G_OBJECT (c), "value", tmp, NULL);
	g_free (tmp);

	g_signal_handlers_unblock_by_func (G_OBJECT (c), value_changed_cb, NULL); 
}

