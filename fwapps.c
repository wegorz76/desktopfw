/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwapps.c
*/

#include <fwcommon.h>
#include <fwpacket.h>
#include <fwapps.h>

#define BUFSIZE     1024

/* ---------------------------------------------------------------------------- */
static GHashTable *HTApps                 = NULL;
static gint        cfgline                = 0;
static GTimeVal    last_check             = { 0, 0 };
static FWAppsStatus appstatus             = FWAPPS_CHANGED;

/* ---------------------------------------------------------------------------- */
/* Return a checksum for application *path, very simple algoritm */
gint              fwapps_get_checksum(gchar *path) {
	FILE  *in;
	gint   checksum;

	if ( (in = fopen(path, "rb")) == NULL ) {
		fw_warning("Can't open application [%s]", path);
		return -1;
	}
	else {
		gint rd, i;
		guchar buf[1024];
		checksum = 0;
		while (!feof(in)) {
			rd = fread(buf, 1, 1024, in);
			if (rd > 0) {
				for (i = 0; i < rd; i++)
					checksum += ((gint) buf[i] * (gint) buf[i]);
			}
		}
		fclose(in);
	}
	return checksum;
}

/* Return an application *path size */
glong             fwapps_get_size(gchar *path) {
	struct stat buf;
	if ( stat(path, &buf) == -1 ) {
		return -1;
	}
	return (glong) buf.st_size;
}

/* Return an application *path mtime */
glong             fwapps_get_mtime(gchar *path) {
	struct stat buf;
	if ( stat(path, &buf) == -1 ) {
		return -1;
	}
	return (glong) buf.st_mtime;
}

/* ---------------------------------------------------------------------------- */
static void       fwapps_ht_free_key(gpointer data) {
	FW_FREEN(data);
}

static void       fwapps_ht_free_value(gpointer data) {
	FWAppsEntry *ae = (FWAppsEntry *) data;
	FW_FREEN(ae->path);
	FW_FREEN(ae);
}

/* ---------------------------------------------------------------------------- */
gboolean          fwapps_add_path(gchar *path, glong mtime, glong size, gint chksum) {
	FWAppsEntry *ae;
	gint currchksum = fwapps_get_checksum(path);
	if (currchksum == -1) {
		return FALSE;
	}

	ae = g_new(FWAppsEntry, 1);
	ae->path       = g_strdup(path);
	ae->mtime      = mtime;
	ae->currmtime  = fwapps_get_mtime(path);
	ae->size       = size;
	ae->currsize   = fwapps_get_size(path);
	ae->chksum     = chksum;
	ae->currchksum = fwapps_get_checksum(path);
	ae->status     = TRUE;
	if (ae->mtime != ae->currmtime ||
			ae->size != ae->currsize ||
			ae->chksum != ae->currchksum)
		ae->status = FALSE;

	fw_message3("Apps Entry [%s], mtime [%ld], size [%ld], checksum [%x]\n", path,
				ae->currmtime, ae->currsize, ae->currchksum);

	g_hash_table_insert(HTApps, g_strdup(path), (gpointer) ae);
	return TRUE;
}

/* ---------------------------------------------------------------------------- */
gboolean          fwapps_add_path_and_commit(gchar *path) {
	FWAppsEntry *ae;
	gint currchksum = fwapps_get_checksum(path);
	if (currchksum == -1) {
		return FALSE;
	}

	ae = g_new(FWAppsEntry, 1);
	ae->path       = g_strdup(path);
	ae->currmtime  = fwapps_get_mtime(path);
	ae->mtime      = ae->currmtime;
	ae->currsize   = fwapps_get_size(path);
	ae->size       = ae->currsize;
	ae->currchksum = fwapps_get_checksum(path);
	ae->chksum     = ae->currchksum;
	ae->status     = TRUE;

	fw_message3("Apps Entry [%s], mtime [%ld], size [%ld], checksum [%x]\n", path,
				ae->currmtime, ae->currsize, ae->currchksum);

	g_hash_table_insert(HTApps, g_strdup(path), (gpointer) ae);
	return TRUE;
}

/* ---------------------------------------------------------------------------- */
gboolean          fwapps_commit_path(gchar *path) {
	FWAppsEntry *ae;

	if ((ae = g_hash_table_lookup(HTApps, (gconstpointer) path)) == NULL) {
		fw_warning("No key [%s] in apps hash table", path);
		return FALSE;
	}
	ae->mtime      = ae->currmtime;
	ae->size       = ae->currsize;
	ae->chksum     = ae->currchksum;

	return TRUE;
}

/* ---------------------------------------------------------------------------- */
gboolean          fwapps_remove_path(gchar *path) {
	return g_hash_table_remove(HTApps, (gpointer) path);
}

/* ---------------------------------------------------------------------------- */
gint              fwapps_read(void) {
	FILE *in;
	FWCfgSection section;
	gchar buf[BUFSIZE];
	gchar path[257];
	glong mtime, size;
	gint  chksum;
	gint num;
	
	FW_DEBUG_INFO3();

	// Read apps file
	if ( (in = fopen(FWAPPS, "r") ) == NULL ) {
		fw_error("Can't open [%s]. Exiting.\n", FWAPPS);
		exit(-1);
	}

	if (HTApps) {
		g_hash_table_destroy(HTApps);
		HTApps = NULL;
	}
	HTApps = g_hash_table_new_full(g_str_hash, g_str_equal,
								   fwapps_ht_free_key, fwapps_ht_free_value);
	
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
		if ( g_strcasecmp(buf, "[apps]") == 0 )
			section = APPS;

		if (section == APPS) {
			fw_message2("section: APPS");
			num = sscanf(buf, "%256s %ld %ld %x", path, &mtime, &size, &chksum);

			if (num != 4) continue;

			fw_message3("scanned: %d", num);
			fw_message2("path: [%s], mtime: [%ld], size: [%ld], chksum [%d]", path, mtime, size, chksum);
			fwapps_add_path(path, mtime, size, chksum);
		}
	}
	fclose(in);

	return 0;
}

/* ---------------------------------------------------------------------------- */
static void       fwapps_print_app(gpointer key, gpointer value, gpointer user_data) {
	FILE *out = (FILE *) user_data;
	FWAppsEntry *ae = (FWAppsEntry *) value;

	fprintf(out, "%-64s %-11ld %-10ld %x \n", ae->path, ae->mtime, ae->size, ae->chksum);
}

void              fwapps_print(FILE *out) {
	FW_DEBUG_INFO3();

	fprintf(out, "# This file is generated automaticaly.\n");
	fprintf(out, "# DON'T MODIFY IT BY HAND UNLESS YOU KNOW WHAT YOU'RE DOING!\n\n");
	fprintf(out, "# fwapps settings\n");
	fprintf(out, "# %-62s %-11s %-10s %s \n", "path", "mtime", "size", "chksum");
	fprintf(out, "[apps]\n");

	g_hash_table_foreach(HTApps, fwapps_print_app, out);
}


/* ---------------------------------------------------------------------------- */
gboolean          fwapps_write(void) {
	FILE *out;

	FW_DEBUG_INFO3();

	if ( rename(FWAPPS, FWAPPSOLD) ) {
		fw_error("Can't rename %s -> %s. Exiting.\n", FWAPPS, FWAPPSOLD);
	}

	if ( (out = fopen(FWAPPS, "w") ) == NULL ) {
		fw_error("Can't open [%s]. Exiting.\n", FWAPPS);
	}
	fwapps_print(out);

	fclose(out);
	return TRUE;
}

/* ---------------------------------------------------------------------------- */
static void       fwapps_check_application(gpointer key, gpointer value, gpointer user_data) {
	FWAppsEntry *ae = (FWAppsEntry *) value;

	ae->currmtime  = fwapps_get_mtime(ae->path);
	ae->currsize   = fwapps_get_size(ae->path);

	/* Count a checksum only if size or mtime changed */
	if ( (ae->currmtime != ae->mtime) || (ae->currsize != ae->size) ) {
		ae->currchksum = fwapps_get_checksum(ae->path);
		appstatus = FWAPPS_CHANGED;
		fw_message3("App [%s] CHANGED", ae->path);
		return;
	}
}

FWAppsStatus      fwapps_check() {
	GTimeVal tv;

	g_get_current_time(&tv);
	fw_message3("Last check time [%ld] : current time [%ld], interval [%d]\n",
				last_check.tv_sec, tv.tv_sec, FWAPPS_CHECKTIME_INTERVAL);
	
	if ( (tv.tv_sec - last_check.tv_sec) < FWAPPS_CHECKTIME_INTERVAL)
		return appstatus;
	
	last_check.tv_sec = tv.tv_sec;
	last_check.tv_usec = tv.tv_usec;
	
	appstatus = FWAPPS_NOT_CHANGED;
	g_hash_table_foreach(HTApps, fwapps_check_application, NULL); //appstatus may change here
	
	return appstatus;
}

/* ---------------------------------------------------------------------------- */
inline guint      fwapps_get_nr() {
	return g_hash_table_size(HTApps);
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/*! Callback for APPS command
*/

gchar            *_fwapps_get_cbs() {
	return g_strconcat( "Available APPS commands:\n"
						" APPS CHECK\n",
						" APPS STATUS\n",
						" APPS ADDCOMMITED application\n",
						" APPS COMMIT application\n",
						" APPS REMOVE application\n",
						NULL );
}

gchar            *fwapps_cb_apps(gchar **strv) {
	gchar *retstr;

	FW_DEBUG_INFO3();

	if (g_strv_length(strv) == 1) {
		/* Show available APP commands */
		return _fwapps_get_cbs();
	}

	if ( !strcmp(strv[1], "CHECK") )            retstr = fwapps_cb_apps_check(strv);
	else if ( !strcmp(strv[1], "STATUS") )      retstr = fwapps_cb_apps_status(strv);
	else if ( !strcmp(strv[1], "ADDCOMMITED") ) retstr = fwapps_cb_apps_addcommited(strv);
	else if ( !strcmp(strv[1], "COMMIT") )      retstr = fwapps_cb_apps_commit(strv);
	else if ( !strcmp(strv[1], "REMOVE") )      retstr = fwapps_cb_apps_remove(strv);
	else
		retstr = _fwapps_get_cbs();

	return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for APPS CHECK command
*/

gchar            *fwapps_cb_apps_check(gchar **strv) {
	gchar *retstr;

	FW_DEBUG_INFO3();

	if ( fwapps_check() == FWAPPS_NOT_CHANGED )
		retstr = g_strdup("APPS NOT CHANGED\n");
	else
		retstr = g_strdup("APPS CHANGED\n");
	
	return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for APPS STATUS command
*/

static void       _fwapps_report_status(gpointer key, gpointer value, gpointer user_data) {
	FWAppsEntry *ae = (FWAppsEntry *) value;
	GString *s = (GString *) user_data;

	gint stat_mtime, stat_size;

	ae->currmtime  = fwapps_get_mtime(ae->path);
	ae->currsize   = fwapps_get_size(ae->path);

	stat_mtime = (ae->currmtime != ae->mtime);
	stat_size  = (ae->currsize != ae->size);
	/* Count a checksum only if size or mtime changed */
	if ( stat_mtime || stat_size ) {
		ae->currchksum = fwapps_get_checksum(ae->path);

		g_string_append_printf(s, "[%s]: [CHANGED: MTIME=%d SIZE=%d CHKSUM=%d]\n",
							   ae->path, stat_mtime, stat_size,
							   (ae->currchksum != ae->chksum) );
	} else {
		g_string_append_printf(s, "[%s]: [%s]\n", ae->path, "NOT CHANGED");
	}
}

gchar            *fwapps_cb_apps_status(gchar **strv) {
	GString *s;

	FW_DEBUG_INFO3();

	s = g_string_new("");
	g_hash_table_foreach(HTApps, _fwapps_report_status, s);
	return g_string_free(s, FALSE);
}

/* ---------------------------------------------------------------------------- */
/*! Callback for APPS ADDCOMMITED application command
*/
gchar            *fwapps_cb_apps_addcommited(gchar **strv) {
	gchar *retstr;

	FW_DEBUG_INFO3();

	if (g_strv_length(strv) != 3) {
		retstr = g_strdup_printf("ERROR: APPS ADDCOMMITED path\n");
	} else {
		if ( fwapps_add_path_and_commit(strv[2]) ) {
			retstr = g_strdup_printf("OK: APPLICATION [%s] ADDED and COMMITED\n", strv[2]);
			fwapps_write();
		}
		else
			retstr = g_strdup_printf("ERROR: APPLICATION [%s] NOT ADDED and NOT COMMITED!\n", strv[2]);
	}

	return retstr;
}


/* ---------------------------------------------------------------------------- */
/*! Callback for APPS COMMIT application command
*/
gchar            *fwapps_cb_apps_commit(gchar **strv) {
	gchar *retstr;

	FW_DEBUG_INFO3();

	if (g_strv_length(strv) != 3) {
		retstr = g_strdup_printf("ERROR: APPS COMMIT path\n");
	} else {
		if ( fwapps_commit_path(strv[2]) ) {
			retstr = g_strdup_printf("OK: APPLICATION [%s] COMMITED\n", strv[2]);
			fwapps_write();
		}
		else
			retstr = g_strdup_printf("ERROR: APPLICATION [%s] NOT COMMITED!\n", strv[2]);
	}

	return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for APPS REMOVE application command
*/
gchar            *fwapps_cb_apps_remove(gchar **strv) {
	gchar *retstr;

	FW_DEBUG_INFO3();

	if (g_strv_length(strv) != 3) {
		retstr = g_strdup_printf("ERROR: APPS REMOVE path\n");
	} else {
		if ( fwapps_remove_path(strv[2]) ) {
			retstr = g_strdup_printf("OK: APPLICATION [%s] REMOVED\n", strv[2]);
			fwapps_write();
		}
		else
			retstr = g_strdup_printf("ERROR: APPLICATION [%s] NOT REMOVED!\n", strv[2]);
	}
	
	return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Register APPS callback
 */
void              fwapps_register_cbs() {
	FW_DEBUG_INFO3();

	fw_command_add_callback("APPS",     fwapps_cb_apps);
}

