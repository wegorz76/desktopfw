/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWPROC_H
#define __FWPROC_H

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
#define BUFSIZE 1024

#define FWPROCDIR    "/proc"
#define FWPROCNETTCP "/proc/net/tcp"
#define FWPROCNETUDP "/proc/net/udp"


/* ---------------------------------------------------------------------------- */
/* FWPidExe garray will contain relation pid->execname */
typedef struct {
  gint           pid;
  gchar         *exe;
} FWPidExeEntry;

/* ---------------------------------------------------------------------------- */
FWPidExeEntry    *fwproc_pidexe_entry_add(gint pid, gchar *exe);
gboolean          fwproc_pidexe_entry_free(gint idx);
void              fwproc_pidexe_free(void);
gint              fwproc_pidexe_sortfn(gconstpointer a, gconstpointer b);
void              fwproc_pidexe_scan(void);
gint              fwproc_pidexe_fprintf(FILE *out);
gint              fwproc_pidexe_find(gint pid);

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
typedef struct {
  //  FWProto        proto;
  gint           sl;
  guint32        local_address;
  guint32        rem_address;
  gint           local_port;
  gint           rem_port;
  gint           st;             //state
  gulong         txq;            //tx_queue
  gulong         rxq;            //rx_queue
  gint           tr;             //timer run
  gulong         tm_when;        //timer when
  gulong         retrnsmt;       //retr
  gint           uid;
  gint           timeout; 
  gulong         inode;
  gint           pid;            //process id
  gint           fd;             //file descriptor
  gchar         *application;
} FWProcNetEntry;


/* ---------------------------------------------------------------------------- */
gint              fwproc_netarray_len(FWProto proto);
FWProcNetEntry   *fwproc_netentry_add(FWProto proto,
				      gint sl,
				      gchar *local_address, 
				      gint   local_port, 
				      gchar *rem_address, 
				      gint   rem_port,
				      gint   state,
				      gulong txq, 
				      gulong rxq, 
				      gint   timer_run, 
				      gulong tm_when, 
				      gulong retr, 
				      gint   uid, 
				      gint   timeout, 
				      gulong inode);

gboolean          fwproc_netentry_free(GArray *connarray, gint idx);
void              fwproc_netentries_free(FWProto proto);

FWProcNetEntry   *fwproc_netentry_find_by_inode(FWProto proto, gulong inode);
gint              fwproc_netentry_sortfn(gconstpointer a, gconstpointer b);
gint              fwproc_netentries_process(FWProto);

void              fwproc_netentry_fprintf(FILE *out, FWProto proto, gint state, gint idx);
gint              fwproc_netentries_fprintf(FILE *out, FWProto proto, gint state);

gchar            *fwproc_netentries_tcpapp_getnew(guint32 srcip, guint32 dstip, gint sport, gint dport);
/* ---------------------------------------------------------------------------- */
typedef struct {
  gulong  inode;
  FWProto proto;
  gchar  *application;
} FWInodeSocket;

void              fwproc_scan_procdir(void);
void              fwproc_rescan_all(void);

#endif 
