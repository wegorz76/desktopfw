/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwproc.c
*/

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
static GArray  *FWPidExe        = NULL;

static GArray  *FWTcpConn       = NULL;
static GArray  *FWUdpConn       = NULL;

/* ---------------------------------------------------------------------------- */
/*! Add FWPidExeEntry to FWPidExe garray.
 */
FWPidExeEntry    *fwproc_pidexe_entry_add(gint pid, gchar *exe)
{
  FWPidExeEntry *pe;

  FW_DEBUG_INFO3();

  pe = g_new(FWPidExeEntry, 1);

  /* Pid */
  pe->pid = pid;
  fw_message3("Pid: %d", pe->pid);

  /* Exe */
  pe->exe = g_strdup(exe);
  fw_message3("Exe: %s", pe->exe);

  g_array_append_val(FWPidExe, pe);

  return pe;
}

/* ---------------------------------------------------------------------------- */
/*! Remove FWPidExeEntry from GArray (index)
 */
gboolean          fwproc_pidexe_entry_free(gint idx)
{
  FWPidExeEntry *pe;

  FW_DEBUG_INFO3();

  if ( (pe = g_array_index(FWPidExe, FWPidExeEntry*, idx)) == NULL ) {
    fw_warning("FWPidExeEntry entry == NULL!");
    return FALSE;
  }

  FW_FREEN(pe->exe);
  FW_FREEN(pe);
  g_array_remove_index(FWPidExe, idx);

  return TRUE;
}

/* ---------------------------------------------------------------------------- */
/*! Free FWPidExe garray and its all elements.
 */
void              fwproc_pidexe_free(void)
{
  gint i, len;

  FW_DEBUG_INFO3();
  
  len = FWPidExe->len;
  for (i = 0; i < len; i++) {
    fwproc_pidexe_entry_free(0);
  }
  g_array_free(FWPidExe, TRUE);

  FWPidExe = NULL;
}

/* ---------------------------------------------------------------------------- */
/*! FWPidExe garray sort function.
 \note Function is not required - files /proc/PIDS are in correct order when
 readdir is performed.
*/
gint              fwproc_pidexe_sortfn(gconstpointer a, gconstpointer b)
{
  FWPidExeEntry *x = (FWPidExeEntry *) ((GArray *)a)->data;
  FWPidExeEntry *y = (FWPidExeEntry *) ((GArray *)b)->data;
  fw_message3("SORT %d, %d, %d", x->pid, y->pid, x->pid - y->pid);
  return x->pid - y->pid;
}

/* ---------------------------------------------------------------------------- */
/*! Scan /proc directory to gather process information like pid and exe.
  For each entry in /proc/PIDNAME check symlink /proc/PIDNAME/exe
 */
void              fwproc_pidexe_scan(void)
{
  GDir          *procdir;
  gchar         *pidname;
  gchar         *execlink;
  gchar          buf[BUFSIZE];
  FWPidExeEntry *pe;

  FW_DEBUG_INFO3();

  /* Prepare FWPidExe array */
  if (FWPidExe) {
    fw_message3("Freeing current pidexe array");
    fwproc_pidexe_free();
  }
  FWPidExe = g_array_new(FALSE, FALSE, sizeof(FWPidExeEntry *));

  if ( (procdir = g_dir_open( FWPROCDIR, 0, NULL )) == NULL )
    g_error("Can't open %s directory", FWPROCDIR);

  do {
    pidname = (gchar *) g_dir_read_name(procdir);
    if (pidname == NULL) break;

    fw_message3("PidName:   [%s]", pidname);

    if (pidname && pidname[0] >= '0' && pidname[0] <= '9') {
      execlink = g_strdup_printf("/proc/%s/exe", pidname);
      bzero(buf, BUFSIZE);
      readlink(execlink, buf, BUFSIZE-1);
      g_free(execlink);

      pe = fwproc_pidexe_entry_add( atoi(pidname), buf );
      fw_message3("PidExe [pid=%d, exe=%s]", pe->pid, pe->exe);
    }
    fw_message3("====================");
  }
  while (pidname);

  g_dir_close(procdir);

  /* Sort using pid as a key */
  //  g_array_sort(FWPidExe, (GCompareFunc) fwproc_pidexe_sortfn);
}

/* ---------------------------------------------------------------------------- */
/*! Print to out stream. 
  \note Only for diagnostic purposes.
*/
gint              fwproc_pidexe_fprintf(FILE *out)
{
  FWPidExeEntry *pe;
  gint i, len;
  
  len = FWPidExe->len;
  for (i = 0; i < len; i++) {
    pe = g_array_index(FWPidExe, FWPidExeEntry*, i);
    fprintf(out, "%6d: pid [%6d], exe [%s]\n", i, pe->pid, pe->exe);
  } 
  return len;
}

/* ---------------------------------------------------------------------------- */
/*! Return i-th FWPidExeEntry element from array.
 \note It is required in \i fwutils_find_elem_in_sorted_garray function.
*/
gpointer          fwproc_pidexe_elemfn(GArray *array, gint i)
{
  FWPidExeEntry *pe;

  //  pe = g_array_index( (FWPidExe *) array, FWPidExeEntry*, i );

  pe = g_array_index( array, FWPidExeEntry*, i );
  return pe;
}

/*! Compare function.
*/
gint              fwproc_pidexe_comparefn(gconstpointer a, gconstpointer e)
{
  FWPidExeEntry *x = (FWPidExeEntry *) a;
  gint        elem = (gint) e;

  g_message("X:    %d", x->pid);
  g_message("ELEM: %d", elem);
  return x->pid - elem;
}

/*! Return \i pid index from FWPidExe garray or \i -1 if pid isn't found in FWPidExe garray.
 */
gint              fwproc_pidexe_find(gint pid)
{
  gint i;

  i = fwutils_find_elem_in_sorted_garray(FWPidExe, fwproc_pidexe_elemfn, fwproc_pidexe_comparefn, (gpointer) pid);

  return i;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
gint              fwproc_netarray_len(FWProto proto)
{
  if (proto == PROTO_TCP)
    return FWTcpConn->len;
  else if (proto == PROTO_UDP)
    return FWUdpConn->len;
  return 0;
}

/* ---------------------------------------------------------------------------- */
/*! Add FWProcNetEntry to FWTcpConn or FWUdpConn garray.
 */
FWProcNetEntry   *fwproc_netentry_add(FWProto proto,
				      gint    sl,
				      gchar  *local_address, 
				      gint    local_port, 
				      gchar  *rem_address, 
				      gint    rem_port,
				      gint    state,
				      gulong  txq, 
				      gulong  rxq, 
				      gint    timer_run, 
				      gulong  tm_when, 
				      gulong  retr, 
				      gint    uid, 
				      gint    timeout, 
				      gulong  inode)
{
  FWProcNetEntry *e;
  guchar *addr;

  FW_DEBUG_INFO3();

  e = g_new(FWProcNetEntry, 1);

  /* Slot number */
  e->sl = sl;
  fw_message3("Slot: %d", e->sl);
  
  /* Local address */
  sscanf(local_address, "%X", &e->local_address);
  addr = (guchar *) &e->local_address;
  fw_message3("Local  address: %d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3] );
  e->local_address = ntohl(e->local_address); //change to host byte order

  /* Remote address */
  sscanf(rem_address, "%X", &e->rem_address);
  addr = (guchar *) &e->rem_address;
  fw_message3("Remote address: %d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3] );
  e->rem_address = ntohl(e->rem_address);
  
  /* Local port */
  e->local_port = htons(local_port);
  fw_message3("Local port: %d", ntohs(e->local_port));

  /* Remote port */
  e->rem_port = htons(rem_port);
  fw_message3("Remote port: %d", ntohs(e->rem_port));

  /* State */
  e->st = state;
  fw_message3("State: %d [%s]", e->st, TxtTcpState[e->st]);
 
  /* txq, rxq */
  e->txq = txq;
  e->rxq = rxq;
  fw_message3("txq [%ld] :: rxq [%ld]", e->txq, e->rxq);

  /* timer run, tm_when, retr */
  e->tr       = timer_run;
  e->tm_when  = tm_when;
  e->retrnsmt = retr;
  fw_message3("timer run [%d] :: tm when [%ld] :: retr [%ld]", e->tr, e->tm_when, e->retrnsmt);

  /* uid */
  e->uid = uid;
  fw_message3("uid [%d]", e->uid);

  /* timeout */
  e->timeout = timeout;
  fw_message3("timeout [%d]", e->timeout);

  /* inode */
  e->inode = inode;
  fw_message3("inode [%ld]", e->inode);

  /* pid */
  /* application */
  e->application = NULL;

  fw_message3("=============");

  if (proto == PROTO_TCP) {
    g_array_append_val(FWTcpConn, e);
  } else if (proto == PROTO_UDP) {
    g_array_append_val(FWUdpConn, e);
  }
  return e;
}

/* ---------------------------------------------------------------------------- */
/*! Free FWProcNetEntry idx-element from \i connarray garray.
 */
gboolean          fwproc_netentry_free(GArray *connarray, gint idx)
{
  FWProcNetEntry *e;

  FW_DEBUG_INFO3();

  if ( (e = g_array_index(connarray, FWProcNetEntry*, idx)) == NULL ) {
    fw_warning("FWProcNetEntry entry == NULL!");
    return FALSE;
  }

  FW_FREEN(e->application);
  FW_FREEN(e);
  g_array_remove_index(connarray, idx);

  return TRUE;
}

/* ---------------------------------------------------------------------------- */
/*! Free \i FWTcpConn or \i FWUdpConn garray.
 */
void              fwproc_netentries_free(FWProto proto)
{
  gint i;
  gint len;

  if (proto == PROTO_TCP) {
    len = FWTcpConn->len;
    for (i = 0; i < len; i++) {
      fwproc_netentry_free(FWTcpConn, 0);
    }
    g_array_free(FWTcpConn, TRUE);
  }
  else if (proto == PROTO_UDP) {
    len = FWUdpConn->len;
    for (i = 0; i < len; i++) {
      fwproc_netentry_free(FWUdpConn, 0);
    }
    g_array_free(FWUdpConn, TRUE);
  }
  else return;
}

/* ---------------------------------------------------------------------------- */
/*! Find FWProcNetEntry by inode.
 */
FWProcNetEntry   *fwproc_netentry_find_by_inode(FWProto proto, gulong inode)
{
  FWProcNetEntry *e;
  GArray *arr;
  gint len, lo, hi, p;

  if (proto == PROTO_TCP) arr = FWTcpConn;
  else if (proto == PROTO_UDP) arr = FWUdpConn;
  else return NULL;

  len = arr->len;

  if (!len)
    return NULL; //no entries found on tcp or udp arrays
  lo = 0;
  hi = len;

  do {
    p = (lo + hi) / 2;

    e = g_array_index(arr, FWProcNetEntry*, p);
    fw_message3("*** Len [%d], p [%d], lo [%d], hi [%d], find inode [%ld], curr inode [%ld] ***", 
	      len, p, lo, hi, inode, e->inode);
    if ((guint) inode == (guint) e->inode) {
      fw_message3("*** FOUND !!! p [%d], lo [%d], hi [%d], inode [%ld] ***", p, lo, hi, inode);
      return e;
    }
    fw_message3("*** VAL [%ld] ***", (inode - e->inode));
    if (inode < e->inode) {
      hi = p;
    } else {
      lo = p;
    }
    fw_message3("*** NEW RANGE: lo [%d], hi [%d] ***", lo, hi);
  }
  while (inode != e->inode && (hi - lo > 1));

  if (lo == 0 && hi == 1) {
    p = (lo + hi) / 2;

    e = g_array_index(arr, FWProcNetEntry*, p);
    fw_message3("*** Len [%d], p [%d], lo [%d], hi [%d], find inode [%ld], curr inode [%ld] ***", 
	      len, p, lo, hi, inode, e->inode);
    if ((guint) inode == (guint) e->inode) {
      fw_message3("*** FOUND !!! p [%d], lo [%d], hi [%d], inode [%ld] ***", p, lo, hi, inode);
      return e;
    }
  }
  
  return NULL;
}

/* ---------------------------------------------------------------------------- */
/*! Comparision function for garray sort.
 */
gint              fwproc_netentry_sortfn(gconstpointer a, gconstpointer b)
{
  FWProcNetEntry *x = (FWProcNetEntry *) ((GArray *)a)->data;
  FWProcNetEntry *y = (FWProcNetEntry *) ((GArray *)b)->data;
  fw_message3("SORT %ld, %ld, %ld", x->inode, y->inode, x->inode - y->inode);
  return x->inode - y->inode;
}

/* ---------------------------------------------------------------------------- */
/*! Process /proc/net/tcp or /proc/net/udp.
  \param proto - Which garray will be processed.
*/
gint              fwproc_netentries_process(FWProto proto)
{
  FILE  *in;
  gchar  buf[BUFSIZE];

  gint num;
  gchar local_address[128], rem_address[128], more[512];
  gint sl, local_port, rem_port, state, timer_run, uid, timeout;
  gulong txq, rxq, tm_when, retr, inode;

  FW_DEBUG_INFO3();
  
  if (proto != PROTO_TCP && proto != PROTO_UDP)
    return -1;

  /* Process PROTO_TCP - prepare array for TCP connections */
  if (proto == PROTO_TCP) {
    if (FWTcpConn) {
      fw_message3("Freeing current array of tcp connections");
      fwproc_netentries_free(PROTO_TCP);
    }
    FWTcpConn = g_array_new(FALSE, FALSE, sizeof(FWProcNetEntry *));
  } 
  /* Process PROTO_UDP - prepare array for UDP connections */
  else if (proto == PROTO_UDP) {
    if (FWUdpConn) {
      fw_message3("Freeing current array of udp connections");
      fwproc_netentries_free(PROTO_UDP);
    }
    FWUdpConn = g_array_new(FALSE, FALSE, sizeof(FWProcNetEntry *));
  }

  if ( proto == PROTO_TCP && (in = fopen(FWPROCNETTCP, "rb")) == NULL )
    g_error("Can't open [%s] file. Exiting.\nError: %s", FWPROCNETTCP, g_strerror(errno));
  else if ( proto == PROTO_UDP && (in = fopen(FWPROCNETUDP, "rb")) == NULL )
    g_error("Can't open [%s] file. Exiting.\nError: %s", FWPROCNETUDP, g_strerror(errno));

  //skip first line
  fgets(buf, BUFSIZE-1, in);

  while (!feof(in)) {
    fgets(buf, BUFSIZE-1, in);
    if (feof(in)) 
      continue;

    g_strchug(buf);
    g_strchomp(buf);
    fw_message3("input line: [%s]", buf);

    num = sscanf(buf,
    "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
		 &sl, local_address, &local_port, rem_address, &rem_port, &state,
		 &txq, &rxq, &timer_run, &tm_when, &retr, &uid, &timeout, &inode, more);

    if (num != 15) 
      continue;

    fwproc_netentry_add(proto,
			sl,
			local_address, local_port, rem_address, rem_port,
			state,
			txq, rxq, 
			timer_run, tm_when, retr, 
			uid, timeout, inode);		     
  }

  fclose(in);

  /* Sort arrays by inode key */
  if (proto == PROTO_TCP) 
    g_array_sort(FWTcpConn, (GCompareFunc) fwproc_netentry_sortfn);
  else if (proto == PROTO_UDP) 
    g_array_sort(FWUdpConn, (GCompareFunc) fwproc_netentry_sortfn);

  return 0;
}

/* ---------------------------------------------------------------------------- */
/*! Print FWProcNetEntry to out stream. 
  \note For diagnostic purposes.
*/
void              fwproc_netentry_fprintf(FILE *out, FWProto proto, gint state, gint idx)
{
  FWProcNetEntry *e;
  gchar *laddr, *raddr;

  if (proto == PROTO_TCP) {
    e = g_array_index(FWTcpConn, FWProcNetEntry*, idx);
  } else if (proto == PROTO_UDP) {
    e = g_array_index(FWUdpConn, FWProcNetEntry*, idx);
  }
  else return;

  /* Print all connections if state == 0, otherwise only connections with state */
  if (e->st != state && state != 0)
    return;

  fprintf(out, "Slot [%d]\n", e->sl);

  /* Local address:port <-> Remote address:port */
  laddr = fw_nethostip_strnew( htonl(e->local_address) );
  raddr = fw_nethostip_strnew( htonl(e->rem_address) );
  fprintf(out, "Local: %s:%d <-> Remote: %s:%d\n", laddr, ntohs(e->local_port), raddr, ntohs(e->rem_port) );

  /* State */
  fprintf(out, "State: %d [%s], ", e->st, TxtTcpState[e->st]);
  
  /* txq, rxq */
  fprintf(out, "txq [%ld]::rxq [%ld], ", e->txq, e->rxq);

  /* timer run, tm_when, retr */
  fprintf(out, "trun [%d], twhen [%ld], retr [%ld], ", e->tr, e->tm_when, e->retrnsmt);

  /* uid */
  fprintf(out, "uid [%d], ", e->uid);

  /* timeout */
  fprintf(out, "timeout [%d], ", e->timeout);

  /* inode */
  fprintf(out, "inode [%ld]\n", e->inode);

  /* pid, application */
  if (e->application)
    fprintf(out, "pid [%d] [%s]\n", e->pid, e->application);

  fprintf(out, "--------------------------------------\n");
}

/* ---------------------------------------------------------------------------- */
/*! Print \i FWTcpConn or \i FWUdpConn garray to out stream. 
  \note For diagnostic purposes.
*/
gint              fwproc_netentries_fprintf(FILE *out, FWProto proto, gint state)
{
  gint i;
  gint l;

  if (proto == PROTO_TCP) {
    l = FWTcpConn->len;
    for (i = 0; i < l; i++) 
      fwproc_netentry_fprintf(out, PROTO_TCP, state, i);
  } 
  else if (proto == PROTO_UDP) {
    l = FWUdpConn->len;
    for (i = 0; i < l; i++) 
      fwproc_netentry_fprintf(out, PROTO_UDP, 0, i);
  }

  return l;
}

/* ---------------------------------------------------------------------------- */
/*! Return an allocated string contains application name for tcp association.
*/
gchar            *fwproc_netentries_tcpapp_getnew(guint32 srcip, guint32 dstip, gint sport, gint dport)
{
  gchar *app = NULL;

  FWProcNetEntry *e;
  gint len, i;

  len = FWTcpConn->len;

  for (i = 0; i < len; i++) {
    e = g_array_index(FWTcpConn, FWProcNetEntry*, i);
    if ( srcip == e->local_address     &&
	 dstip == e->rem_address       &&
	 sport == ntohs(e->local_port) &&
	 dport == ntohs(e->rem_port) ) {
      if (e->application) 
	app = g_strdup( e->application );
      else 
	app = g_strdup("*");
      break;
    }
  }
  return app;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
void              fwproc_scan_fddir(gchar *pidname, gchar *fdname)
{
  GDir        *fddir;
  gchar       *fdlink;
  struct stat  fdstat;
  gchar        buf[BUFSIZE];
  gchar       *execlink;

  if ( (fddir = g_dir_open( fdname, 0, NULL )) == NULL ) 
    return;
  
  fw_message3("FdDir:  [%s]", fdname);
  chdir(fdname);
  do {
    fdlink = (gchar *) g_dir_read_name(fddir);
    if (!fdlink) break;

    fw_message3("FdLink: [%s]", fdlink);
    stat(fdlink, &fdstat);
    if ( S_ISSOCK( fdstat.st_mode ) ) { 
      fw_message3("Socket: [%s], Inode [%ld]", fdlink, fdstat.st_ino);

      FWProcNetEntry *e = fwproc_netentry_find_by_inode(PROTO_TCP, fdstat.st_ino);
      if (e == NULL) 
	e = fwproc_netentry_find_by_inode(PROTO_UDP, fdstat.st_ino);
      if (e) {
	execlink = g_strdup_printf("/proc/%s/exe", pidname);
	bzero(buf, BUFSIZE);
	readlink(execlink, buf, BUFSIZE-1);
	g_free(execlink);
	if (e->application == NULL) {
	  e->pid = atoi(pidname);
	  e->application = g_strdup_printf("%s", buf);
	  fw_message3("InodeSocket [pid=%d, %ld, %s]", e->pid, e->inode, e->application);
	}
      }
    } 
  }
  while (fddir);
  g_dir_close(fddir);

  return;
}

/* ---------------------------------------------------------------------------- */
/*! Scan /proc directory to gather process information like pid and exe.
  For each entry in /proc/PIDNAME scan it /proc/PIDNAME/fd/...
 */
void              fwproc_scan_procdir(void)
{
  GDir   *procdir;
  gchar  *pidname, *fdname;

  if ( (procdir = g_dir_open( FWPROCDIR, 0, NULL )) == NULL )
    g_error("Can't open %s directory", FWPROCDIR);

  do {
    pidname = (gchar *) g_dir_read_name(procdir);
    if (pidname == NULL) break;

    fw_message3("PidName:   [%s]", pidname);

    if (pidname && pidname[0] >= '0' && pidname[0] <= '9') {
      fdname = g_strdup_printf("/proc/%s/fd", pidname);
      fwproc_scan_fddir(pidname, fdname);
      g_free(fdname);
    }
    fw_message3("====================");
  }
  while (pidname);

  g_dir_close(procdir);
}

/* ---------------------------------------------------------------------------- */
/*! Process /proc directory to collect (refresh) connections and applications 
 */
void              fwproc_rescan_all(void) {
  fwproc_pidexe_scan();
  fwproc_netentries_process(PROTO_TCP);
  fwproc_netentries_process(PROTO_UDP);
  fwproc_scan_procdir();
}
