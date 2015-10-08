/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwconfig.c
*/

#include <fwcommon.h>
#include <fwpacket.h>

#define USERBUFSIZE 17
#define HOSTBUFSIZE 65
#define BUFSIZE     1024
/* ---------------------------------------------------------------------------- */
static gchar    fw_username[USERBUFSIZE];
static gchar    fw_password[USERBUFSIZE];
static gchar    fw_host[HOSTBUFSIZE];
static gint     fw_port                   = 32123; 
static FWAction fw_default_input_action   = ACTION_ASK;
static FWAction fw_default_output_action  = ACTION_ASK;
static gint     cfgline                   = 0;

/* ---------------------------------------------------------------------------- */
gint              fwconfig_read(void) {
  FILE *in;
  FWCfgSection section;
  gchar buf[BUFSIZE];
  gchar key[33], value[17];
  gint num;

  FW_DEBUG_INFO3();

  // Read configuration file
  if ( (in = fopen(FWCONFIG, "r") ) == NULL ) {
    fw_error("Can't open [%s]. Exiting.\n", FWCONFIG);
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

    // Detect sections
    if ( g_strcasecmp(buf, "[settings]") == 0 ) 
      section = SETTINGS;

    if (section == SETTINGS) {

      fw_message2("section: SETTINGS");

      num = sscanf(buf, "%32s = %64s", key, value);

      if (num != 2) continue;

      fw_message3("scanned: %d", num);
      fw_message2("key: [%s], value: [%s]", key, value);

      // variables
      if ( g_strcasecmp(key, "username") == 0 )
	strncpy(fw_username, value, USERBUFSIZE-1);
      else if ( g_strcasecmp(key, "password") == 0 )
	strncpy(fw_password, value, USERBUFSIZE-1);
      else if ( g_strcasecmp(key, "host") == 0 )
	strncpy(fw_host, value, HOSTBUFSIZE-1);
      else if ( g_strcasecmp(key, "port") == 0 )
	fw_port = atoi(value);
      else if ( g_strcasecmp(key, "default_input_action") == 0 )
	{
	  fw_default_input_action = fwpacket_get_action_by_name(value);
	  if ( fw_default_input_action == ACTION_UNKNOWN )
	    fw_error("Unknown input action: [%s] in configuration file (line = %d)\n", value, cfgline);
	}
      else if ( g_strcasecmp(key, "default_output_action") == 0 )
	{
	  fw_default_output_action = fwpacket_get_action_by_name(value);
	  if ( fw_default_output_action == ACTION_UNKNOWN )
	    fw_error("Unknown output action: [%s] in configuration file (line = %d)\n", value, cfgline);
	}
    }
  }
  fclose(in);
  
  return 0;
}

/* ---------------------------------------------------------------------------- */
void              fwconfig_print(FILE *out) {

  FW_DEBUG_INFO3();

  fprintf(out, "# This file is generated automaticaly.\n");
  fprintf(out, "# DON'T MODIFY IT BY HAND UNLESS YOU KNOW WHAT YOU'RE DOING!\n\n");
  fprintf(out, "# fwconfig settings\n");
  fprintf(out, "# key           \t=       value\n");
  fprintf(out, "[settings]\n");
  fprintf(out, "username\t\t=\t%s\n", fw_username);
  fprintf(out, "password\t\t=\t%s\n", fw_password);
  fprintf(out, "host\t\t\t=\t%s\n", fw_host);
  fprintf(out, "port\t\t\t=\t%d\n", fw_port);
  fprintf(out, "default_input_action\t=\t%s\n", TxtAction[fw_default_input_action]);
  fprintf(out, "default_output_action\t=\t%s\n", TxtAction[fw_default_output_action]);

}


/* ---------------------------------------------------------------------------- */
gboolean          fwconfig_write(void) {
  FILE *out;

  FW_DEBUG_INFO3();

  if ( rename(FWCONFIG, FWCONFIGOLD) ) {
    fw_error("Can't rename %s -> %s. Exiting.\n", FWCONFIG, FWCONFIGOLD);
  }

  if ( (out = fopen(FWCONFIG, "w") ) == NULL ) {
    fw_error("Can't open [%s]. Exiting.\n", FWCONFIG);
  }
  fwconfig_print(out);

  fclose(out);
  return TRUE;
}

/* ---------------------------------------------------------------------------- */
inline gchar     *fwconfig_get_fw_username() { 
  return fw_username;
}

inline gchar     *fwconfig_get_fw_password() {
  return fw_password;
}

inline gchar     *fwconfig_get_fw_host() {
  return fw_host;
}

inline gint       fwconfig_get_fw_port() {
  return fw_port;
}

inline gint       fwconfig_get_fw_default_in_action() {
  return fw_default_input_action;
}

inline gint       fwconfig_get_fw_default_out_action() {
  return fw_default_output_action;
}

/* ---------------------------------------------------------------------------- */
gboolean          fwconfig_set_fw_username(gchar *username) {
  if ( strlen(username) < USERBUFSIZE ) {
    strcpy(fw_username, username); 
    return TRUE;
  }
  fw_warning("Can't change username (length(username) >= %d)", USERBUFSIZE);
  return FALSE;
}

gboolean          fwconfig_set_fw_password(gchar *password) {
  if ( strlen(password) < USERBUFSIZE ) {
    strcpy(fw_password, password); 
    return TRUE;
  }
  fw_warning("Can't change password (length(password) >= %d)", USERBUFSIZE);
  return FALSE;
}

gboolean          fwconfig_set_fw_host(gchar *host) {
  if ( strlen(host) < HOSTBUFSIZE ) {
    strcpy(fw_host, host); 
    return TRUE;
  }
  fw_warning("Can't change host (length(host) >= %d)", HOSTBUFSIZE);
  return FALSE;
}

gboolean          fwconfig_set_fw_port(gint port) {
  if ( port >= 1024 && port <= 65535 ) {
    fw_port = port;
    return TRUE;
  }
  fw_warning("Can't change fwport (not in range 1024 <= %d <= 65535)", port);
  return FALSE;
}

gboolean          fwconfig_set_fw_default_in_action(FWAction action) {
  if (IS_FWACTION(action)) {
    fw_default_input_action = action;
    return TRUE;
  }
  fw_warning("Unknown action %d", action);
  return FALSE;
}

gboolean          fwconfig_set_fw_default_out_action(FWAction action) {
  if (IS_FWACTION(action)) {
    fw_default_output_action = action;
    return TRUE;
  }
  fw_warning("Unknown action %d", action);
  return FALSE;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/*! Callback for CONFIG command
*/

gchar            *_fwconfig_get_cbs() {
  return g_strconcat( "Available CONFIG commands:\n" 
		      " CONFIG LIST\n",
		      " CONFIG SET variable value\n",
		      NULL );
}

gchar            *fwconfig_cb_config(gchar **strv) {
  gchar *retstr;

  FW_DEBUG_INFO3();

  if (g_strv_length(strv) == 1) {
    /* Show available CONFIG commands */
    return _fwconfig_get_cbs();
  } 

  if ( !strcmp(strv[1], "LIST") )             retstr = fwconfig_cb_config_list(strv);
  else if ( !strcmp(strv[1], "SET") )         retstr = fwconfig_cb_config_set(strv);
  else 
    retstr = _fwconfig_get_cbs();

  return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for CONFIG LIST command
*/

gchar            *fwconfig_cb_config_list(gchar **strv) {
  GString *s;

  FW_DEBUG_INFO3();

  s = g_string_new("");
  g_string_append_printf(s, "username              = %s\n", fw_username);
  g_string_append_printf(s, "password              = %s\n", "********");
  g_string_append_printf(s, "host                  = %s\n", fw_host);
  g_string_append_printf(s, "port                  = %d\n", fw_port);
  g_string_append_printf(s, "default_input_action  = %s\n", TxtAction[fw_default_input_action]);
  g_string_append_printf(s, "default_output_action = %s\n", TxtAction[fw_default_output_action]);

  return g_string_free(s, FALSE);
}

/* ---------------------------------------------------------------------------- */
/*! Callback for RULES ADD command
*/

gchar            *fwconfig_cb_config_set(gchar **strv) {
  gboolean st;
  gchar *retstr, *var, *val; 

  FW_DEBUG_INFO3();

  if ( g_strv_length(strv) != 4 ) {
    return g_strdup("usage:\n"
		    " CONFIG SET variable value\n");
  }

  var = strv[2];
  val = strv[3];

  // SET username
  if ( !strcmp(var, "username") ) {
    st = fwconfig_set_fw_username( val );
    if (!st) 
      return g_strdup_printf("ERROR: Can't change username, max length = %d\n", USERBUFSIZE-1);
    else
      retstr = g_strdup("OK: username set\n");
  } 
  // SET password
  else if ( !strcmp(var, "password") ) {
    st = fwconfig_set_fw_password( val );
    if (!st) 
      return g_strdup_printf("ERROR: Can't change password, max length = %d\n", USERBUFSIZE-1);
    else
      retstr = g_strdup("OK: password set\n");
  } 
  // SET host
  else if ( !strcmp(var, "host") ) {
    guint32 hostip;
    gint    mask;
    gchar  *hoststr = g_strdup_printf("%s/32", val);

    // check is a hoststr a valid ip address
    st = fw_get_hostip_mask(hoststr, &hostip, &mask);
    FW_FREEN(hoststr);
    if (!st)
      return g_strdup_printf("ERROR: Can't parse host ip address [%s]\n", val);

    st = fwconfig_set_fw_host( val );
    if (!st) 
      return g_strdup_printf("ERROR: Can't change host ip, max length = %d\n", HOSTBUFSIZE-1);
    else
      retstr = g_strdup("OK: host ip set. fwdaemon restart required!\n");
  } 
  // SET port
  else if ( !strcmp(var, "port") ) {
    st = fwconfig_set_fw_port( atoi(val) );
    if (!st) 
      return g_strdup("ERROR: Can't change port, must be in range <1024,65535>\n");
    else
      retstr = g_strdup("OK: port set. fwdaemon restart required!\n");
  } 
  // SET default_input_action
  else if ( !strcmp(var, "default_input_action") ) {
    FWAction action = fwpacket_get_action_by_name(val);
    st = fwconfig_set_fw_default_in_action( action );
    if (!st) 
      return g_strdup_printf("ERROR: Can't change input action to [%s]\n", val);
    else
      retstr = g_strdup("OK: default input action set\n");
  } 
  // SET default_output_action
  else if ( !strcmp(var, "default_output_action") ) {
    FWAction action = fwpacket_get_action_by_name(val);
    st = fwconfig_set_fw_default_out_action( action );
    if (!st) 
      return g_strdup_printf("ERROR: Can't change output action to [%s]\n", val);
    else
      retstr = g_strdup("OK: default output action set\n");
  } 
  else 
    retstr = g_strdup_printf("ERROR: unknown variable [%s]\n", var);

  if ( !strncmp(retstr, "OK", 2) )
    fwconfig_write();

  return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Register CONFIG callback
 */
void              fwconfig_register_cbs() {
  FW_DEBUG_INFO3();

  fw_command_add_callback("CONFIG",    fwconfig_cb_config);
}


