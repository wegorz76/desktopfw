/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwrules.c
*/

#include <fwcommon.h>
#include <fwpacket.h>

/* ---------------------------------------------------------------------------- */
G_LOCK_DEFINE     (FWRulesLock);
G_LOCK_DEFINE     (FWRulesEntryLock);

static GArray   *fw_rules                  = NULL;
static gint     cfgline                   = 0;

gchar *TxtCfgEntry[]  = { "UNKNOWN", "PERM", "SESS" };

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/*! Add an entry to one of fw_rules array.
 */
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
				    gchar      *application)
{
  FWRuleEntry *e;

  FW_DEBUG_INFO3();
			 
  e = g_new(FWRuleEntry, 1);

  e->type = type;
  
  // Interface
  e->iface = g_strdup(iface);
    
  // Direction
  e->direction = fwpacket_get_direction_by_name(direction);
  if ( e->direction == DIR_UNKNOWN )
    g_error("Section [rules], line [%d]: Unknown direction in config file [%s]", cfgline, direction);
    
  // Protocol
  e->proto = fwpacket_get_proto_by_name(proto);
  if ( e->proto == PROTO_UNKNOWN )
    g_error("Section [rules], line [%d]: Unknown protocol in rules file [%s]", cfgline, proto);

  // src hosts
  e->srchosts = g_strdup(srchosts);
  if ( fw_get_hostip_mask(srchosts, &e->hsrcip, &e->hsrcmask) == FALSE )
    fw_error("Can't parse cidr [%s]\n", srchosts);

  if ( srcport >= 0 && srcport < 65536 )
    e->srcport = srcport;
  else
    g_error("Section [entries], line [%d]: Src port is not between 0-65535 [%d]", cfgline, srcport);

  // dst hosts
  e->dsthosts = g_strdup(dsthosts);
  if ( fw_get_hostip_mask(dsthosts, &e->hdstip, &e->hdstmask) == FALSE )
    fw_error("Can't parse cidr [%s]\n", dsthosts);

  if ( dstport >= 0 && dstport < 65536 )
    e->dstport = dstport;
  else
    g_error("Section [entries], line [%d]: Dst port is not between 0-65535 [%d]", cfgline, dstport);

  // Action
  e->action = fwpacket_get_action_by_name(action);
  if (e->action == ACTION_UNKNOWN)
    g_error("Section [rules], line [%d]: Unknown action in rules file [%s]", cfgline, action);

  // Application
  e->application = g_strdup(application);

  if (e->type == ENTRY_PERMANENT) 
    e->lastactivity = 0;
  else {
    GTimeVal tv;
    g_get_current_time(&tv);
    e->lastactivity = tv.tv_sec;
  }

  // Statistic counter
  e->statcnt = 0;

  G_LOCK(FWRulesLock);
  // if position == -1 insert at the fw_rules end
  if (position == -1)
    g_array_insert_val(fw_rules, fw_rules->len, e);
  else 
    g_array_insert_val(fw_rules, position, e);
  G_UNLOCK(FWRulesLock);

  return e;
}

/* ---------------------------------------------------------------------------- */
gboolean          fwrules_entry_move(gint from, gint to)
{
  //  gint elen    = FWCfgEntries[arraynr]->len;

  FW_DEBUG_INFO3();

  // Range checking for 'from' and 'to' arguments
  /*
  if (from == to) {
    g_warning("Can't move entry. Argument 'from' == 'to'.");
    return FALSE;
  }

  if (from < 0 || from >= elen) {
    g_warning("Can't move entry. Argument 'from' is beyond entries range [from = %d, array range = <0,%d>].", 
	      from, elen-1);
    return FALSE;
  }

  if (to < 0 || to >= elen) {
    g_warning("Can't move entry. Argument 'to' is beyond entries range [to = %d, array range = <0,%d>].", 
	      to, elen-1);
    return FALSE;
  }

  FWCfgEntry *f = g_array_index(FWCfgEntries[arraynr], FWCfgEntry*, from); 
  g_array_remove_index_fast(FWCfgEntries[arraynr], from);
  g_array_insert_val(FWCfgEntries[arraynr], to, f);  
  */
  return TRUE;
}

/* ---------------------------------------------------------------------------- */
gboolean          fwrules_entry_free(gint num)
{
  FWRuleEntry *e;

  FW_DEBUG_INFO3();

  if ( (e = g_array_index(fw_rules, FWRuleEntry*, num)) == NULL ) {
    g_warning("FWRuleEntry entry == NULL!");
    return FALSE;
  }

  FW_FREEN(e->iface);
  FW_FREEN(e->srchosts);
  FW_FREEN(e->dsthosts);
  FW_FREEN(e->application);
  FW_FREEN(e);
  
  G_LOCK(FWRulesEntryLock);
  g_array_remove_index(fw_rules, num);
  G_UNLOCK(FWRulesEntryLock);

  return TRUE;
}

/* ---------------------------------------------------------------------------- */
void              fwrules_entries_free()
{
  gint elen;
  int i;

  FW_DEBUG_INFO3();

  G_LOCK(FWRulesLock);
  elen = fw_rules->len;
  for (i = 0; i < elen; i++) {
    fwrules_entry_free(0);
  }
  g_array_free(fw_rules, TRUE);
  G_UNLOCK(FWRulesLock);
}

/* ---------------------------------------------------------------------------- */
gboolean          fwrules_entry_delete(gint num)
{
  gint elen = fw_rules->len;

  FW_DEBUG_INFO3();

  // Range checking for 'from' and 'to' arguments
  if (num < 0 || num >= elen) {
    g_warning("Can't delete rule entry. Argument 'num' is beyond entries range [from = %d, array range = <0,%d>].", 
	      num, elen-1);
    return FALSE;
  }

  // Checking ok, user can delete entry
  fwrules_entry_free(num);

  return TRUE;
}

/* ---------------------------------------------------------------------------- */
void              fwrules_entries_print()
{
  gint len, i;

  FW_DEBUG_INFO3();

  G_LOCK(FWRulesLock);
  len = fw_rules->len;
  printf("array len %d\n", fw_rules->len);
  for (i = 0; i < len; i++) {
    FWRuleEntry *e = g_array_index(fw_rules, FWRuleEntry*, i);

    printf(" * type: %s\n", TxtCfgEntry[e->type]);
    printf(" * iface: %s\n", e->iface);
    printf(" * direction: %s\n", TxtDirection[e->direction]);
    printf(" * protocol: %s\n", TxtProto[e->proto]);
    printf(" * srcport: %d\n", e->srcport);
    printf(" * dstport: %d\n", e->dstport);
    printf(" * action: %s\n", TxtAction[e->action]);
    printf(" * application: %s\n", e->application);
    //    printf(" * pid: %d\n", e->pid);
    printf(" * statcnt: %ld\n", e->lastactivity);
    printf(" * statcnt: %ld\n", e->statcnt);
    
    printf("\n");
  }  
  G_UNLOCK(FWRulesLock);
}

/* ---------------------------------------------------------------------------- */
FWAction          fwrules_entry_get_action(gint        ifaceidx,
					   FWDirection direction,
					   FWProto     proto,
					   guint32     srcip,
					   gint        sport,
					   guint32     dstip,
					   gint        dport,
					   gchar      *application)
{
  FWAction action;
  gint     len = fw_rules->len;
  gint     i;
  gchar   *rulestr;
  GTimeVal tv;

  FW_DEBUG_INFO3();

  printf("* get action *\n");
  
  G_LOCK(FWRulesLock);

  for (i = 0; i < len; i++) {
    FWRuleEntry *e = g_array_index(fw_rules, FWRuleEntry*, i);
    rulestr = fwrules_strnew_printf(e);
    printf("====\n* current rule: %s\n", rulestr);
    FW_FREEN(rulestr);

    gint devidx = fwdev_find_ifaceidx(e->iface);
    if ( !devidx ) {
      printf("  ** NOT MATCHED: interface defined in rules not found [%d]\n", devidx);
      continue; //interface defined in rules not found 
    }
    
    if (devidx != ifaceidx) {
      printf("  ** NOT MATCHED: interface idx not matched [%d <> %d]\n", devidx, ifaceidx);
      continue;
    }
    fw_message3("iface equal [%s,%s]", fwdev_string_by_idx(devidx), fwdev_string_by_idx(ifaceidx));
    
    if (e->direction != direction) {
      printf("  ** NOT MATCHED: directions not matched [%d <> %d]\n", e->direction, direction);
      continue;
    }
    fw_message3("direction equal [%s,%s]", TxtDirection[e->direction], TxtDirection[direction]);

    if (e->proto != proto) {
      printf("  ** NOT MATCHED: protocols not equal [%d <> %d]\n", e->proto, proto);
      continue;
    }
    fw_message3("protocol equal [%s,%s]", TxtProto[e->proto], TxtProto[proto]);
    
    /* src host */
    if ( strcmp(e->srchosts, "0/0") ) {
      gchar *t = fw_nethostip_strnew( htonl(e->hsrcip) );
      gchar *u = fw_nethostip_strnew( htonl(srcip) );
      printf("  ** SRCIP [%s], SRC CLIENT HOST [%s]\n", t, u);      
      FW_FREEN(t);
      FW_FREEN(u);

      guint32 h1 = e->hsrcip;
      guint32 h2 = srcip;
      guint32 mask = ((2 << (e->hsrcmask-1)) - 1) << (32 - e->hsrcmask);
      h1 = h1 & mask;
      h2 = h2 & mask;
      printf("  ** mask %x, h1 = %x, h2 = %x\n", mask, h1, h2);
      if (h1 != h2) {
	printf("  ** NOT MATCHED: host/mask not equal [%x <> %x]\n", h1, h2);
	continue;
      }      
    }

    /* dst host */
    if ( strcmp(e->dsthosts, "0/0") ) {
      gchar *t = fw_nethostip_strnew( htonl(e->hdstip) );
      gchar *u = fw_nethostip_strnew( htonl(dstip) );
      printf("  ** DSTIP [%s], DST CLIENT HOST [%s]\n", t, u);      
      FW_FREEN(t);
      FW_FREEN(u);

      guint32 h1 = e->hdstip;
      guint32 h2 = dstip;
      guint32 mask = ((2 << (e->hdstmask-1)) - 1) << (32 - e->hdstmask);
      h1 = h1 & mask;
      h2 = h2 & mask;
      printf("  ** mask %x, h1 = %x, h2 = %x\n", mask, h1, h2);
      if (h1 != h2) {
	printf("  ** NOT MATCHED: host/mask not equal [%x <> %x]\n", h1, h2);
	continue;
      }      
    }
    if (e->srcport != sport && e->srcport != 0) {
      printf("  ** NOT MATCHED: src ports differs [%d <> %d] or rules port <> 0\n", e->srcport, sport);
      continue;
    }
    fw_message3("src port equal [%d,%d] or cfg port == 0", e->srcport, sport);

    if (e->dstport != dport && e->dstport != 0) {
      printf("  ** NOT MATCHED: dst ports differs [%d <> %d] or rules port <> 0\n", e->dstport, dport);
      continue;
    }
    fw_message3("dst port equal [%d,%d] or cfg port == 0", e->dstport, dport);

    /* application to check ! */
    if ( application && strcmp(application, "*") && strcmp(e->application, application)) {
      printf("  ** NOT MATCHED: applications differs [%s <> %s]\n", e->application, application);
      continue;
    }
    g_get_current_time(&tv);
    e->lastactivity = tv.tv_sec;    
    e->statcnt++;

    printf("  ** MATCHED **\n");
    
    G_UNLOCK(FWRulesLock);
    return e->action;
  }
  G_UNLOCK(FWRulesLock);

  if ( direction == DIR_OUTGOING ) {
    action = fwconfig_get_fw_default_out_action();
    printf(" *** Outgoing rule not found. Use default out action: %d\n", action);
    return action;
  } else {
    action = fwconfig_get_fw_default_in_action();
    printf(" *** Incoming rule not found. Use default in action: %d\n", action);
  }

  return action;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
gint              fwrules_read(void) {
  FILE *in;
  FWCfgSection section;
  gchar buf[BUFSIZE];
  gchar iface[17], direction[9], proto[9], srchosts[19], dsthosts[19], application[256], action[8];
  gint sport, dport;
  gint num;

  FW_DEBUG_INFO3();

  // Free FWUSERcfg if necessary
  if (fw_rules) {
    fw_message3("Freeing fw_rules array...");
    g_message("Freeing fw_rules array...");
    fwrules_entries_free();
    fw_rules = NULL;
  } 

  // Read configuration file
  if ( (in = fopen(FWRULES, "r") ) == NULL ) {
    fw_error("Can't open [%s]. Exiting.\n", FWRULES);
    exit(-1);
  }

  cfgline = 0;
  while (!feof(in)) {
    fgets(buf, BUFSIZE-1, in);
    if (feof(in)) 
      continue;

    cfgline++;

    g_strchug(buf);
    g_strchomp(buf);
    fw_message3("input line: [%s]", buf);

    // Skip comments
    if (buf[0] == '#') continue; 

    // Detect section rules
    if ( g_strcasecmp(buf, "[rules]") == 0 ) 
      {
	section = RULES;
	fw_message2("Allocating array fw_rules");
	fw_rules = g_array_new(FALSE, FALSE, sizeof(FWRuleEntry *));
      }

    // Parse configuration
    if (section == RULES) {
      fw_message2("section: RULES");
      
      num = sscanf(buf, "%15s %8[a-z] %8[a-z] %18s %d %18s %d %8[A-Z] %256s", 
		   iface, direction, proto, srchosts, &sport, dsthosts, &dport, action, application);
      if (num != 9) continue;
      
      fw_message3("scanned: %d", num);
      fw_message2("iface: [%s], dir: [%s], proto [%s], srchosts [%s], sport [%d], dsthosts[%s], dport[%d], action[%s], application[%s]", 
		  iface, direction, proto, srchosts, sport, dsthosts, dport, action, application);

      fwrules_entry_add(fw_rules->len,
      			ENTRY_PERMANENT,
      			iface, direction, proto, srchosts, sport, dsthosts, dport, action, application);
    }
  }
  fclose(in);
  
  return 0;
}

/* ---------------------------------------------------------------------------- */
gchar            *fwrules_strnew_printf(FWRuleEntry *e) {
  GString *s = g_string_new("");
  g_string_append_printf(s, "%s\t%s\t%s\t%-18s\t%d\t%-18s\t%d\t%s\t%-33s\t%ld\n", 
			 e->iface, 
			 TxtDirection[e->direction],
			 TxtProto[e->proto],
			 e->srchosts,
			 e->srcport,
			 e->dsthosts,
			 e->dstport,
			 TxtAction[e->action],
			 e->application,
			 e->statcnt
			 );
  return g_string_free(s, FALSE);
}

/* ---------------------------------------------------------------------------- */
void              fwrules_print(FILE *out)
{
  gint i;

  G_LOCK(FWRulesLock);
  for (i = 0; i < fw_rules->len; i++) {
    FWRuleEntry *e = g_array_index(fw_rules, FWRuleEntry*, i);
    if (e->type == ENTRY_PERMANENT) {
      fprintf(out, "%s\t%s\t%s\t%-18s\t%d\t%-18s\t%d\t%s\t%-33s\n", 
	      e->iface, 
	      TxtDirection[e->direction],
	      TxtProto[e->proto],
	      e->srchosts,
	      e->srcport,
	      e->dsthosts,
	      e->dstport,
	      TxtAction[e->action],
	      e->application
	      );
    }
  }
  G_UNLOCK(FWRulesLock);
}

/* ---------------------------------------------------------------------------- */
gboolean          fwrules_write(void) {
  FILE *out;

  FW_DEBUG_INFO3();

  // Write only if a rules array isn't empty
  if (!fw_rules) return FALSE;

  if ( rename(FWRULES, FWRULESOLD) ) {
    fw_error("Can't rename %s -> %s. Exiting.\n", FWRULES, FWRULESOLD);
  }

  if ( (out = fopen(FWRULES, "w") ) == NULL ) {
    fw_error("Can't open [%s]. Exiting.\n", FWRULES);
  }

  fprintf(out, "# This file is generated automaticaly.\n");
  fprintf(out, "# DON'T MODIFY IT BY HAND UNLESS YOU KNOW WHAT YOU'RE DOING!\n\n");
  fprintf(out, "# fwdaemon rules\n");
  fprintf(out, "# iface direction       proto   srchost                    srcport dsthost          dstport    action  application\n");
  fprintf(out, "[rules]\n");

  // Write rules
  fwrules_print(out);
  fclose(out);
  
  return TRUE;
}

/* ---------------------------------------------------------------------------- */
void              fwrules_free(void)
{
  fwrules_entries_free();
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/*! Callback for RULES command
*/

gchar            *_fwrules_get_cbs() {
  return g_strconcat( "Available RULES commands:\n" 
		      " RULES LIST\n",
		      " RULES ADD SESS iface dir proto srchost srcport dsthost dstport action application\n",
		      " RULES ADD PERM iface dir proto srchost srcport dsthost dstport action application\n",
		      " RULES REMOVE PERM rule_number\n",
		      " RULES REMOVE SESS rule_number\n",
		      NULL );
}

gchar            *fwrules_cb_rules(gchar **strv) {
  gchar *retstr;

  FW_DEBUG_INFO3();

  if (g_strv_length(strv) == 1) {
    /* Show available RULES commands */
    return _fwrules_get_cbs();
  } 

  if ( !strcmp(strv[1], "LIST") )             retstr = fwrules_cb_rules_list(strv);
  else if ( !strcmp(strv[1], "ADD") )         retstr = fwrules_cb_rules_add(strv);
  else if ( !strcmp(strv[1], "REMOVE") )      retstr = fwrules_cb_rules_remove(strv);
  else 
    retstr = _fwrules_get_cbs();

  return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for RULES LIST command
*/

gchar            *fwrules_cb_rules_list(gchar **strv) {
  GTimeVal tv;
  GString *s;
  FWRuleEntry *e;
  gint sessnr = 0, permnr = 0, currnr = 0;
  gint i;

  FW_DEBUG_INFO3();

  fwrules_remove_timeouted();

  g_get_current_time(&tv);
  s = g_string_new("[RULES currtime=");
  g_string_append_printf(s, "%ld]\n", tv.tv_sec);

  G_LOCK(FWRulesLock);
  for (i = 0; i < fw_rules->len; i++) {
    e = g_array_index(fw_rules, FWRuleEntry*, i);
    if (e->type == ENTRY_PERMANENT) {
      currnr = permnr++;
    } else {
      currnr = sessnr++;
    }
    g_string_append_printf(s, "[%04d] %s\t%s\t%s\t%s\t%-18s\t%d\t%-18s\t%d\t%s\t%-33s\t%ld\t%ld\n", 
			   currnr,
			   TxtCfgEntry[e->type], 
			   e->iface, 
			   TxtDirection[e->direction],
			   TxtProto[e->proto],
			   e->srchosts,
			   e->srcport,
			   e->dsthosts,
			   e->dstport,
			   TxtAction[e->action],
			   e->application,
			   e->lastactivity,
			   e->statcnt
			   );
  }
  G_UNLOCK(FWRulesLock);

  return g_string_free(s, FALSE);
}

/* ---------------------------------------------------------------------------- */
/*! Callback for RULES ADD command
*/

gchar            *fwrules_cb_rules_add(gchar **strv) {
  gchar       *retstr;
  FWRuleType   type;
  FWDirection  dir;
  FWProto      proto;
  FWAction     action;
  guint32      hsrcip;
  gint         hsrcmask;
  gint         sport;
  guint32      hdstip;
  gint         hdstmask;
  gint         dport;

  gchar       *asess, *aiface, *adir, *aproto, *asrchost, *asrcport, *adsthost, *adstport, *aaction, *aapp;
  gint         position, idx;

  FW_DEBUG_INFO3();

  if ( g_strv_length(strv) != 12 ) {
    return g_strdup("usage:\n"
		    " RULES ADD SESS iface dir proto srchost srcport dsthost dstport action application\n"
		    " RULES ADD PERM iface dir proto srchost srcport dsthost dstport action application\n"
		    " note: SESSion entry is inserted at the beginning of the rules, \n"
		    "       PERManent entry at the end\n");
  }

  idx = 2;
  asess    = strv[idx++];
  aiface   = strv[idx++];
  adir     = strv[idx++];
  aproto   = strv[idx++];
  asrchost = strv[idx++];
  asrcport = strv[idx++];
  adsthost = strv[idx++];
  adstport = strv[idx++];
  aaction  = strv[idx++];
  aapp     = strv[idx++];

  if ( !strcmp(asess, "PERM" ) ) {
    type     = ENTRY_PERMANENT;
    position = -1;
  }
  else if ( !strcmp(asess, "SESS" ) ) {
    type = ENTRY_SESSION;
    position = 0;
  }
  else 
    return g_strdup_printf("ERROR: RULES ADD - arg [%s] must be SESS or PERM\n", asess);

  // verify rule arguments
  dir = fwpacket_get_direction_by_name(adir);
  if ( dir == DIR_UNKNOWN )
    return g_strdup_printf("ERROR: RULES ADD - unknown direction [%s]:\n", adir);

  proto = fwpacket_get_proto_by_name(aproto);
  if ( proto == PROTO_UNKNOWN )
    return g_strdup_printf("ERROR: RULES ADD - unknown protocol [%s]:\n", aproto);

  if ( fw_get_hostip_mask(asrchost, &hsrcip, &hsrcmask) == FALSE )
    return g_strdup_printf("ERROR: RULES ADD - can't parse cidr src address [%s]:\n", asrchost);

  sport = atoi(asrcport);
  if ( sport < 0 || sport > 65535 ) 
    return g_strdup_printf("ERROR: RULES ADD - src port [%s] out of range <0,65535>\n", asrcport);

  if ( fw_get_hostip_mask(adsthost, &hdstip, &hdstmask) == FALSE )
    return g_strdup_printf("ERROR: RULES ADD - can't parse cidr dst address [%s]:\n", adsthost);

  dport = atoi(adstport);
  if ( dport < 0 || dport > 65535 ) 
    return g_strdup_printf("ERROR: RULES ADD - dst port [%s] out of range <0,65535>\n", adstport);

  action = fwpacket_get_action_by_name(aaction);
  if (action == ACTION_UNKNOWN)
    return g_strdup_printf("ERROR: RULES ADD - action uknown [%s]\n", aaction);

  if ( strlen(aapp) > 256 )
    return g_strdup_printf("ERROR: RULES ADD - application can't be no longer than 256 characters [%s]\n", aapp);

  fwrules_entry_add( position, type, aiface, adir, aproto, asrchost, sport, adsthost, dport, aaction, aapp );

  retstr = g_strdup("OK: RULES ADD\n");

  fwrules_write();
  
  return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for RULES REMOVE command
*/
gchar            *fwrules_cb_rules_remove(gchar **strv) {
  FWRuleEntry  *e;
  gchar        *retstr = NULL;
  gint          rulenum;
  gint          currnr, sessnr = 0, permnr = 0;
  gint          i;

  if ( g_strv_length(strv) != 4 ) {
    return g_strdup("usage:\n"
		    " RULES REMOVE PERM rule_number\n"
		    " RULES REMOVE SESS rule_number\n");
  }

  FW_DEBUG_INFO3();

  // RULES REMOVE PERM rule_number
  if ( g_strv_length(strv) == 4 && !strcmp(strv[2], "PERM") ) {
    rulenum = atoi(strv[3]);
    for (i = 0; i < fw_rules->len; i++) {
      e = g_array_index(fw_rules, FWRuleEntry*, i);
      if (e->type == ENTRY_PERMANENT)
	currnr = permnr++;
      if (rulenum == currnr) {
	g_array_remove_index(fw_rules, i);
	retstr = g_strdup_printf("OK: Permantent rule [%d, index=%d] removed\n", currnr, i);
	fwrules_write();
	return retstr;
      }	
    }
  }
  else if ( g_strv_length(strv) == 4 && !strcmp(strv[2], "SESS") ) {
    rulenum = atoi(strv[3]);
    
    for (i = 0; i < fw_rules->len; i++) {
      e = g_array_index(fw_rules, FWRuleEntry*, i);
      if (e->type == ENTRY_SESSION)
	currnr = sessnr++;
      if (rulenum == currnr) {
	g_array_remove_index(fw_rules, i);
	retstr = g_strdup_printf("OK: Session rule [%d, index=%d] removed\n", currnr, i);
	return retstr;
      }	
    }
  }

  if (retstr == NULL) 
    retstr = g_strdup_printf("ERROR: Rule number [%04d] not found in %s rules\n", 
			     rulenum, strv[2]);
  return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Register RULES callback
 */
void              fwrules_register_cbs() {
  FW_DEBUG_INFO3();

  fw_command_add_callback("RULES",     fwrules_cb_rules);
}

/* ---------------------------------------------------------------------------- */
/*! Remove timeouted session rules
 */
static GTimeVal last_check = {0, 0};
void              fwrules_remove_timeouted(void) {
  FWRuleEntry *e;
  GTimeVal curr;
  gint i;

  FW_DEBUG_INFO3();

  g_get_current_time(&curr);

  // refresh not so often
  if ( curr.tv_sec - last_check.tv_sec < FWSESSRULE_MINREFRESH ) return;
  else last_check = curr;

  G_LOCK(FWRulesLock);
  // remove session rules after FWSESSRULE_TIMEOUT inactivity
  for (i = 0; i < fw_rules->len; i++) {
    e = g_array_index(fw_rules, FWRuleEntry*, i);
    if (e->type == ENTRY_SESSION && 
	((curr.tv_sec - e->lastactivity) >= FWSESSRULE_TIMEOUT) )      
      g_array_remove_index(fw_rules, i);
  }
  G_UNLOCK(FWRulesLock);
}
