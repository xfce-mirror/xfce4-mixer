#include <stdio.h>
/*
 gcc -o test_vc test_vc.c vc_oss.c -I../include `pkg-config --cflags --libs gtk+-2.0 libxfce4util-1.0` vc.c vcs.c -DUSE_OSS -DHAVE_CONFIG_H -g3 -I..
 */
#include "vc.h"

int main()
{
  GList* devices;
  GList* device_item;
  gchar const* device_name;
  register_vcs ();
  
  devices = vc_get_device_list ();
  if (devices != NULL) {
    device_item = devices;
    while (device_item != NULL) {
      device_name = (gchar const*) device_item->data;
      
      printf(" Device: %s\n", device_name);
      
      device_item = g_list_next (device_item);
    }
    
    vc_free_device_list (devices);
  }
  
  return 0;
}
