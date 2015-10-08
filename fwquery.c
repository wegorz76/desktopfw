/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwquery.c
*/

#include <fwcommon.h>
#include <fwpacket.h>
#include <fwquery.h>
#include <fwnfq.h>

#define BUFSIZE     1024
/* ---------------------------------------------------------------------------- */
static GArray     *Queries                = NULL;
glong              qnum                   = 1;
/* ---------------------------------------------------------------------------- */
/* Add a query to query table */
gboolean          fwquery_add(gchar *iface, FWDirection dir, FWProto proto,
			      gchar *application, guint32 srcip, guint32 dstip, 
			      gint sport, gint dport, guint32 id)
{
  FWQuery *q;
  gint i;

  FW_DEBUG_INFO3();

  // Return false if a query already exists 
  for (i = 0; i < Queries->len; i++) {
    FWQuery *cq = g_array_index(Queries, FWQuery*, i);
    if ( sport == cq->sport &&
	 dport == cq->dport &&
	 srcip == cq->srcip &&
	 dstip == cq->dstip ) {
      printf("Query already exists\n");
      printf("Dropping previous packet id [%d]\n", id);
      g_get_current_time(&cq->instime); //update timer
      fwnfq_drop_packet(cq->packetid);
      cq->packetid = id;
      return FALSE;
    }
  }

  q = g_new(FWQuery, 1);
  q->qnum  = qnum++;
  q->iface = g_strdup(iface);
  q->dir   = dir;
  q->proto = proto;
  q->application = g_strdup(application);
  q->srcip = srcip;
  q->dstip = dstip;
  q->sport = sport;
  q->dport = dport;

  g_get_current_time(&q->instime);
  g_array_append_val(Queries, q);

  q->packetid = id;

  return TRUE;
}

/* ---------------------------------------------------------------------------- */
/* Free a fwquery struct */
void              fwquery_free(FWQuery *q)
{
  FW_DEBUG_INFO3();

  FW_FREEN(q->iface);
  FW_FREEN(q->application);
  FW_FREEN(q);
}

/* ---------------------------------------------------------------------------- */
void              fwquery_print(FILE *out) {
  gint i;

  FW_DEBUG_INFO3();
  
  for (i = 0; i < Queries->len; i++) {
    fprintf(out, "Query: %d\n", i);
  }
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/*! Callback for QUERY command
*/

gchar            *_fwquery_get_cbs() {
  return g_strconcat( "Available QUERY commands:\n" 
		      " QUERY LIST\n",
		      " QUERY ADD PERM qnumber [ANYWHERE]\n",
		      " QUERY ADD SESS qnumber [ANYWHERE]\n",
		      NULL );
}

gchar            *fwquery_cb_query(gchar **strv) {
  gchar *retstr;

  FW_DEBUG_INFO3();

  if (g_strv_length(strv) == 1) {
    /* Show available QUERY commands */
    return _fwquery_get_cbs();
  } 

  if ( !strcmp(strv[1], "LIST") )                 retstr = fwquery_cb_query_list(strv);
  else if ( !strcmp(strv[1], "ADD") )             retstr = fwquery_cb_add(strv);
  else 
    retstr = _fwquery_get_cbs();

  return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for QUERY LIST command
*/
gchar            *fwquery_cb_query_list(gchar **strv) {
  GString *s;
  gint i;
  GTimeVal tv;

  FW_DEBUG_INFO3();

  g_get_current_time(&tv);
  s = g_string_new("[QUERIES currtime=");
  g_string_append_printf(s, "%ld]\n", tv.tv_sec);

  //remove timeouted queries first
  for (i = 0; i < Queries->len; i++) {
    FWQuery *q = g_array_index(Queries, FWQuery*, i);
    if (tv.tv_sec - q->instime.tv_sec > FWQUERY_TIMEOUT) {
      fwnfq_drop_packet(q->packetid);
      g_array_remove_index(Queries, i);      
    }
  }

  for (i = 0; i < Queries->len; i++) {
    FWQuery *q = g_array_index(Queries, FWQuery*, i);
    gchar *srcstr = fw_nethostip_strnew( htonl(q->srcip) );
    gchar *dststr = fw_nethostip_strnew( htonl(q->dstip) );
    g_string_append_printf(s, "Q[%ld] ",              q->qnum);
    g_string_append_printf(s, "IFACE[%s] ",           q->iface);
    g_string_append_printf(s, "DIR[%s] ",             TxtDirection[q->dir]);
    g_string_append_printf(s, "SRCIP[%s] ",           srcstr);
    g_string_append_printf(s, "DSTIP[%s] ",           dststr);
    g_string_append_printf(s, "SPORT[%d] ",           q->sport);
    g_string_append_printf(s, "DPORT[%d] ",           q->dport);
    g_string_append_printf(s, "APP[%s] ",             q->application);
    g_string_append_printf(s, "INSTIME[%ld]\n",       q->instime.tv_sec);
    FW_FREEN(srcstr);
    FW_FREEN(dststr);
  }

  return g_string_free(s, FALSE);
}

/* ---------------------------------------------------------------------------- */
/*! Callback for QUERY ADD (PERM|SESS) command
*/
gchar            *fwquery_cb_add(gchar **strv) {
  gint       qnum, i, inspoint;
  gboolean   anywhere = FALSE;
  FWRuleType type;

  gchar     *usage = "usage: QUERY ADD PERM qnumber [ANYWHERE]\n"
    "       QUERY ADD SESS qnumber [ANYWHERE]\n";

  FW_DEBUG_INFO3();

  if ( g_strv_length(strv) < 4 )
    return g_strdup(usage);
  
  qnum = atoi(strv[3]);

  if ( !strcmp(strv[2], "PERM") ) {
    type  = ENTRY_PERMANENT;
    inspoint = -1;
  }
  else if ( !strcmp(strv[2], "SESS") ) {
    type = ENTRY_SESSION;
    inspoint = 0;
  }
  else
    return g_strdup(usage);

  //if ANYWHERE is added application at qnumber can connect to any host
  if (g_strv_length(strv) == 5 && !strcmp(strv[4], "ANYWHERE") )
    anywhere = TRUE;

  for (i = 0; i < Queries->len; i++) {
    FWQuery *q = g_array_index(Queries, FWQuery*, i);
    if (q->qnum == qnum) {
      printf("query number index = %d\n", i);
      gchar *srcstr = fw_nethostip_mask_strnew( htonl(q->srcip), 32 );
      gchar *dststr = fw_nethostip_mask_strnew( htonl(q->dstip), 32 );  
    
      fwnfq_accept_packet(q->packetid);

      if (anywhere == FALSE) {
	fwrules_entry_add(inspoint,
			  type,
			  q->iface,
			  TxtDirection[q->dir], 
			  TxtProto[q->proto], 
			  srcstr,
			  q->sport,
			  dststr,
			  q->dport,
			  "ACCEPT",
			  q->application);
      } else {
	fwrules_entry_add(inspoint,
			  type,
			  q->iface,
			  TxtDirection[q->dir], 
			  TxtProto[q->proto], 
			  "0/0",
			  0,
			  "0/0",
			  0,
			  "ACCEPT",
			  q->application);
      }
      FW_FREEN(srcstr);
      FW_FREEN(dststr);
      g_array_remove_index(Queries, i);

      if (type == ENTRY_PERMANENT)
	fwrules_write();
    }
  }

  return g_strdup_printf("QUERY NUMBER [%d] ADDED TO RULES\n", qnum);
}


/* ---------------------------------------------------------------------------- */
/*! Register APPS callback
 */
void              fwquery_register_cbs() {

  FW_DEBUG_INFO3();

  if (!Queries) Queries = g_array_new(FALSE, FALSE, sizeof(FWQuery*));

  fw_command_add_callback("QUERY",     fwquery_cb_query);
}

