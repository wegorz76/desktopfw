/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
/*! 
	\file test_fwdev.c 
*/

#include <stdio.h>

#include <fwmacros.h>
#include <fwcommon.h>
#include <fwconfig.h>
#include <fwdev.h>

int main(gint argc, gchar **argv) {
  //  int i; 
  fw_debug_set(5);
  fw_debug_funcs_set(5);
  fw_debug_filepos_set(5);
  fw_debug_off();
  fw_debug_funcs_off();
  fw_debug_filepos_off();

  fw_init(argc, argv);

  int i;
  for (i = 0; i < 1000000; i++)
    fwdev_read();

  fwdev_print();

  //  gchar iface[17];
  //  if ( fwdev_find_iface(2, iface) ) {
  //    printf("Iface nr 2: %s\n", iface);
  //  }
  return 0;
}
