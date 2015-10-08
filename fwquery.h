/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWQUERY_H
#define __FWQUERY_H

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
#define FWQUERY_TIMEOUT 60
#define BUFSIZE 1024

typedef struct {
  glong         qnum;
  gchar        *iface;        
  FWDirection   dir;          
  FWProto       proto;     
  gchar        *application;  
  guint32       srcip;
  guint32       dstip;
  gint          sport;
  gint          dport;
  GTimeVal      instime;        //insert time
  guint32       packetid;       //nfq packet id
} FWQuery;

/* ---------------------------------------------------------------------------- */

/* Add a query to query table */
gboolean          fwquery_add(gchar *iface, FWDirection dir, FWProto proto,
			      gchar *application, guint32 srcip, guint32 dstip, 
			      gint sport, gint dport, guint32 id);

void              fwquery_free(FWQuery *q);

/* Print configuration to out */
void              fwquery_print(FILE *out);

/* ---------------------------------------------------------------------------- */
gchar            *fwquery_cb_query(gchar **strv);
gchar            *fwquery_cb_query_list(gchar **strv);
gchar            *fwquery_cb_add(gchar **strv);
void              fwquery_register_cbs();

#endif 
