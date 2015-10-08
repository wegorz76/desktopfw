/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
/*! 
	\file test_fwapps.c 
*/

#include <stdio.h>

#include <fwmacros.h>
#include <fwcommon.h>
#include <fwapps.h>
//#include <fwproc.h>
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
    printf("%d: %d\n", i, fwapps_read());
    printf("==================================\n");
    //fwapps_print(stdout);
    //fwapps_write();
  }

  //  fwapps_add_path("/usr/bin/ssh", 0, 0, 0);
  //  fwapps_commit_path("/usr/bin/ssh");
  //  fwapps_add_path_and_commit("/bin/ping");
  fwapps_print(stdout);
  fwapps_write();
  
  int a = 0;
  while(1) {
    FWAppsStatus s = fwapps_check();

    printf("Registered apps = %d\n", fwapps_get_nr());
    if (s == FWAPPS_CHANGED) {
      printf("APPS CHANGED\n");
    }
    sleep(1);
    a++;

    if (a == 20) {
      fwapps_commit_path("/bin/ping");
      fwapps_write();
    }

    if (a == 30) {
      fwapps_remove_path("/bin/ping");
      fwapps_write();
    }

  }
  fwapps_check();

  //printf("username:  %s\n", fwconfig_get_fw_username());
  return 0;
}
