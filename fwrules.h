/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWRULES_H
#define __FWRULES_H

#include <fwcommon.h>
#include <fwpacket.h>

/* ---------------------------------------------------------------------------- */
#define FWRULES    "/etc/desktopfw/rules"
#define FWRULESOLD "/etc/desktopfw/rules.old"
#define BUFSIZE    1024

#define FWSESSRULE_TIMEOUT    60
#define FWSESSRULE_MINREFRESH 3
/* ---------------------------------------------------------------------------- */
/* Rules structs end enums */
/* ---------------------------------------------------------------------------- */
typedef enum {
  ENTRY_UNKNOWN,
  ENTRY_PERMANENT,
  ENTRY_SESSION
} FWRuleType;

typedef struct {
  FWRuleType    type;         //PERMANENT (will be save in /etc/desktopfw/rules) or SESSION  
  gchar        *iface;        //interface name
  FWDirection   direction;    //INCOMING or OUTGOING
  FWProto       proto;        //protocol: ICMP, UDP, TCP

  gchar        *srchosts;     //src host(s) in CIDR notation. 
  gint          srcport;      //src port 
  guint32       hsrcip;       //contains src ip address (in host byte order)
  gint          hsrcmask;     //contains src ip mask (in host byte order)

  gchar        *dsthosts;     //dst host(s) in CIDR notation. 
  gint          dstport;      //dst port 
  guint32       hdstip;       //contains dst ip address (in host byte order)
  gint          hdstmask;     //contains dst ip mask (in host byte order)

  FWAction      action;       //ASK, ACCEPT, DROP, REJECT
  gchar        *application;  //path to the application

  glong         lastactivity; 
  glong         statcnt;      //statistic counter (how many time rule was matched)
} FWRuleEntry;

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
FWRuleEntry      *fwrules_entry_add(gint        position,
				    FWRuleType  type,
				    gchar      *iface,
				    gchar      *direction,
				    gchar      *proto,
				    gchar      *srchosts,
				    gint        srcport,
				    gchar      *dsthosts,
				    gint        dstport,
				    gchar      *action,
				    gchar      *application);

gboolean          fwrules_entry_move(gint from, gint to);
gboolean          fwrules_entry_free(gint num);
void              fwrules_entries_free();

FWAction          fwrules_entry_get_action(gint        ifaceidx,
					   FWDirection direction,
					   FWProto     proto,
					   guint32     srcip,
					   gint        sport,
					   guint32     dstip,
					   gint        dport,
					   gchar      *application);

//gboolean    fwrules_entry_delete(GArray *fwarray, gint num);
//void        fwconfig_entries_print(GArray *fwarray);

gint              fwrules_read(void);
gchar            *fwrules_strnew_printf(FWRuleEntry *e);
void              fwrules_print(FILE *out);
gboolean          fwrules_write(void);
void              fwrules_free(void);

/* ---------------------------------------------------------------------------- */
gchar            *fwrules_cb_apps(gchar **strv);
gchar            *fwrules_cb_rules_list(gchar **strv);
gchar            *fwrules_cb_rules_add(gchar **strv);
gchar            *fwrules_cb_rules_remove(gchar **strv);
void              fwrules_register_cbs();

void              fwrules_remove_timeouted(void);
#endif 
