/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
/*! 
	\file test_fwproc.c 
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
  fw_debug_off();
  fw_debug_funcs_off();
  fw_debug_filepos_off();

  fw_init(argc, argv);

  int i;
  for (i = 0; i < 1; i++) {
    fwproc_pidexe_scan();
    fwproc_pidexe_fprintf(stdout);
  }

  int elem = 10;
  i = fwproc_pidexe_find(elem);
  if (i >= 0) {
    printf("PidExeEntry index = %d [elem = %d]\n", i, elem);
  } else {
    printf("Elem [%d] in PidExe NOT FOUND\n", elem);
  }
  
  //  return 0;
  //  int i;
  for (i = 0; i < 100000; i++) {

    fwproc_netentries_process(PROTO_TCP);
    fwproc_netentries_process(PROTO_UDP);
    fwproc_scan_procdir();
    fwproc_netentries_fprintf(stdout, PROTO_TCP, TCP_SYN_SENT); //ESTABLISHED);
    fwproc_netentries_fprintf(stdout, PROTO_TCP, TCP_ESTABLISHED); //ESTABLISHED);
    fwproc_netentries_fprintf(stdout, PROTO_TCP, TCP_LISTEN); //ESTABLISHED);
  
  }

  //fwproc_conn_print(TCP, ESTABLISHED);
  //fwproc_conn_print(TCP, LISTEN);
  fwproc_netentries_fprintf(stdout, PROTO_TCP, TCP_SYN_SENT); //ESTABLISHED);
  fwproc_netentries_fprintf(stdout, PROTO_TCP, TCP_ESTABLISHED); //ESTABLISHED);
  //  g_message("==============");
  //  fwproc_conn_print(UDP, 0); //ESTABLISHED);

  return 0;
}
