/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
/*! 
	\file test_fwconfig.c 
*/

#include <stdio.h>

#include <fwmacros.h>
#include <fwcommon.h>
#include <fwconfig.h>
#include <fwproc.h>
//#include <fwprocnet.h>

int main(gint argc, gchar **argv) {
  //  int i; 
  fw_debug_set(5);
  fw_debug_funcs_set(5);
  fw_debug_filepos_set(5);
  //    fw_debug_off();
  //    fw_debug_funcs_off();
  //    fw_debug_filepos_off();

  fw_init(argc, argv);

  int i;
  for (i = 0; i < 1; i++) {
    printf("%d: %d\n", i, fwconfig_read());
    printf("==================================\n");
    fwconfig_print(stdout);
    fwconfig_write();
  }

  fwconfig_print(stdout);
  fwconfig_write();
  printf("username:  %s\n", fwconfig_get_fw_username());
  printf("password:  %s\n", fwconfig_get_fw_password());
  printf("host:      %s\n", fwconfig_get_fw_host());
  printf("port:      %d\n", fwconfig_get_fw_port());
  printf("inaction:  %d\n", fwconfig_get_fw_default_in_action());
  printf("outaction: %d\n", fwconfig_get_fw_default_out_action());

  return 0;
}
