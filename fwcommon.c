/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */

#include <fwcommon.h>

/*! \file fwcommon.c
  \brief FW common debug definitons and others.
*/

/*! \file fwcommon.c */

/* Commands hash table. Contains relation COMMAND => CALLBACK FUNC. */
GHashTable       *HTCmds                  = NULL;

/* Debug - messaging, warnings and errors */
gint              FWDebugLevel            = 5;     
gint              FWDebugFuncsLevel       = 5;  
gint              FWDebugFilePosLevel     = 5;

G_LOCK_DEFINE     (FWGlobalLock);

/* ---------------------------------------------------------------------------- */
/*! Print debug information about file, line and function currently executed. 
  This function should be called using \a fw_message() macros. 
  \note \a FWDebugFilePosLevel produces informations: 
  0-no info, 1-function, 2-line-function, 
  3-function-line, 4-file-function, 5-file-line-function */
void              fw_filepos_print(gchar *file, gint line, gchar *function)
{
  switch (FWDebugFilePosLevel)
    {
    case 0: return;
    case 1: g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "=[ Func:%s ]=",                   function); return;
    case 2: g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "=[ Line:%d (func:%s) ]=",         line, function); return;
    case 3: g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "=[ Func:%s (line %d) ]=",         function, line); return;
    case 4: g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "=[ File:%s (func:%s) ]=",         file, function); return;
    case 5: g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "=[ File:%s (line %d) func:%s ]=", file, line, function); return;
    default: return;            
    }
}

/* ---------------------------------------------------------------------------- */
/*! Initialize \a FW debug variables.
*/
void              fw_init(gint argc, gchar **argv)
{
  FW_DEBUG_INFO2();

  // Check debugging environment settings
  FWDebugLevel          = FW_ENVINT( FW_ENV_DEBUG_LEVEL,         FWDebugLevel );
  FWDebugFuncsLevel     = FW_ENVINT( FW_ENV_DEBUG_FUNCS_LEVEL,   FWDebugFuncsLevel );
  FWDebugFilePosLevel   = FW_ENVINT( FW_ENV_DEBUG_FILEPOS_LEVEL, FWDebugFilePosLevel );

  //  fw_message2("Set progname to %s", argv[0]);
  g_set_prgname(argv[0]);
}

/* ---------------------------------------------------------------------------- */
/*! Initialize \a command callback hash
*/
void              fw_command_hash_init()
{
  FW_DEBUG_INFO2();
  
  if (!HTCmds) HTCmds = g_hash_table_new( g_str_hash, g_str_equal );
  g_hash_table_insert( HTCmds, "VERSION", (gpointer) fw_command_show_version );
  g_hash_table_insert( HTCmds, "LIST", (gpointer) fw_command_list );
}

/* ---------------------------------------------------------------------------- */
/*! Register a command callback
*/

void              fw_command_add_callback(gchar *cmd, void *callback)
{
  FW_DEBUG_INFO2();

  if (cmd == NULL || callback == NULL) {
    fw_warning("Cmd or callback is NULL\n");
    return;
  }

  g_hash_table_insert( HTCmds, (gpointer) cmd, (gpointer) callback );
}

/* ---------------------------------------------------------------------------- */
/*! Callback for VERSION command
*/
gchar            *fw_command_show_version(gchar **strv) {
  printf("Version: 0.1\n");
  return g_strdup("0.1\n");
}

/* ---------------------------------------------------------------------------- */
/*! Callback for LIST
*/
void             _fw_command_list_cmd(gpointer key, gpointer value, 
				      gpointer user_data) {
  GString *s = (GString *) user_data;
  g_string_append_printf(s, "%s\n", (gchar *) key);
}

gchar            *fw_command_list(gchar **strv) {
  GString *s = g_string_new("Command list:\n----------\n");
 
  g_hash_table_foreach( HTCmds, _fw_command_list_cmd, s );
  g_string_append(s, "----------\n");

  return g_string_free( s, FALSE );
}

/* ---------------------------------------------------------------------------- */
/*! Print all registered commands
*/

void             _fw_command_print_cmd(gpointer key, gpointer value, 
				       gpointer user_data) {
  printf("Cmd: [%s], address = [%x]\n", (gchar *) key, (gint) value);
}

void              fw_command_print_registered_cmds()
{
  FW_DEBUG_INFO2();
  
  if (!HTCmds) return;
  g_hash_table_foreach( HTCmds, _fw_command_print_cmd, NULL );
}

/* ---------------------------------------------------------------------------- */
/*! Call cmd callback
  strv - command read from user 
  retbuf - callback function need to return 
*/
gchar            *fw_command_call(gchar **strv)
{
  gchar *(*func)(gchar **strv);
  gchar *retstr = "";

  FW_DEBUG_INFO2();
  
  if (strv[0] == NULL) {
    fw_warning("Cmd is NULL\n");
    return g_strdup("Cmd is NULL!");
  }

  func = g_hash_table_lookup( HTCmds, strv[0] );

  printf("Call [%s] => [%x]\n", strv[0], (gint) func);
  if (func) {
    fw_message("Call [%s] => [%x]\n", strv[0], (gint) func);
    retstr = func( strv );
  } else {
    retstr = g_strdup_printf("Unknown command [%s]\n", strv[0]);
  }

  return retstr;
}

/* ---------------------------------------------------------------------------- */
gchar            *fw_nethostip_strnew(guint32 hostip) {
  guchar *p = (guchar *) &hostip;
  return g_strdup_printf("%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

/* ---------------------------------------------------------------------------- */
gchar            *fw_nethostip_mask_strnew(guint32 hostip, gint mask) {
  guchar *p = (guchar *) &hostip;
  return g_strdup_printf("%d.%d.%d.%d/%d", p[0], p[1], p[2], p[3], mask);
}

/* ---------------------------------------------------------------------------- */
gboolean          fw_get_hostip_mask(gchar *hoststr, guint32 *hostip, gint *mask) {
  gint cidr[4];

  if ( !strcmp(hoststr, "0/0") ) {
    *hostip = 0;
    *mask   = 0;
    return TRUE;
  }

  gint num = sscanf(hoststr, "%d.%d.%d.%d/%d", 
		    &(cidr[0]), 
		    &(cidr[1]), 
		    &(cidr[2]), 
		    &(cidr[3]), 
		    mask);
  *hostip = cidr[0] << 24 | cidr[1] << 16 | cidr[2] << 8 | cidr[3];
  if (num != 5) return FALSE;

  // range checking
  if ( cidr[0] < 0 || cidr[0] > 255 ) return FALSE;
  if ( cidr[1] < 0 || cidr[1] > 255 ) return FALSE;
  if ( cidr[2] < 0 || cidr[2] > 255 ) return FALSE;
  if ( cidr[3] < 0 || cidr[3] > 255 ) return FALSE;
  if ( *mask < 0 || *mask > 32 ) return FALSE;

  return TRUE;
}
