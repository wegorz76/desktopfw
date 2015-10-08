/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
/*! 
	\file fwdaemon.c 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include <fwmacros.h>
#include <fwcommon.h>
#include <fwconfig.h>
#include <fwapps.h>
#include <fwproc.h>
#include <fwdev.h>
#include <fwquery.h>
#include <fwnfq.h>

void sigsegv_handler(int a) {
  fprintf(stderr, "Sigsegv\n");
  chdir("/tmp");
  abort();
}

int main(int argc, char **argv)
{
  fw_debug_set(5);
  fw_debug_funcs_set(5);
  fw_debug_filepos_set(5);
  fw_debug_off();
  fw_debug_funcs_off();
  fw_debug_filepos_off();

  g_thread_init(NULL);
  if ( !g_thread_supported() ) {
    fw_error("Can't initialize glib threads. Exiting...\n");
  }

  signal(SIGSEGV, sigsegv_handler);

  //initialize callback hash table
  fw_command_hash_init();
  fwconfig_register_cbs();  //CONFIG commands
  fwcomm_register_cbs();    //AUTH command
  fwdev_register_cbs();     //DEV * commands
  fwapps_register_cbs();    //APPS * commands
  fwrules_register_cbs();   //RULES * commands
  fwquery_register_cbs();   //QUERY * commands

  fwdev_read();
  fwconfig_read();
  fwapps_read();
  fwrules_read();
  fwrules_print(stdout);
  fwrules_write();

  /* Collect apps info */
  fwproc_rescan_all();

  //show registered commands
  fw_command_print_registered_cmds();

  fwcomm_thread_create();
  fwnfq_init();
  fwnfq_loop();

  while(1) {
    if ( !fwcomm_thread_is_running() ) break;
    sleep(1);
  }
  return 0;
}

