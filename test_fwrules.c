/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
/*! 
	\file test_fwrules.c 
*/

#include <stdio.h>

#include <fwmacros.h>
#include <fwcommon.h>
#include <fwconfig.h>
#include <fwproc.h>

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
  for (i = 0; i < 1000000; i++) {
    printf("%d\n", fwrules_read());
    printf("==================================\n");
  }

  //  fwrules_entry_add(1, 
  //		    ENTRY_SESSION,
  //		    "lo", "incoming", "tcp", "0/0", 10000, "ACCEPT", "wegorz", "*", "*", 10);
  fwrules_print(stdout);
  fwrules_write();
  //  fwrules_entries_print();
  //  fwrules_entry_print(0);
  //  fwrules_entry_move(1, 0);

  //  printf("Action: %d\n", fwrules_entry_get_action(-2, "eth0", DIR_INCOMING, PROTO_UDP, "0/0", 53, "*"));

  return 0;
}
