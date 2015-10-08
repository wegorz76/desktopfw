/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWAPPS_H
#define __FWAPPS_H

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
// application checking interval
#define FWAPPS_CHECKTIME_INTERVAL 5
#define FWAPPS      "/etc/desktopfw/apps"
#define FWAPPSOLD   "/etc/desktopfw/apps.old"
#define BUFSIZE 1024

typedef enum {
  FWAPPS_NOT_CHANGED = 1,
  FWAPPS_CHANGED,
} FWAppsStatus;

typedef struct {
  gchar        *path;         //path to executable 
  glong         mtime;        //mtime from apps config file
  glong         currmtime;    //current (real) application mtime
  glong         size;         //size from apps config file
  glong         currsize;     //current (real) application size
  gint          chksum;       //checksum from apps config file
  gint          currchksum;   //current (real) checksum
  FWAppsStatus  status;       //FWAPPS_CHANGED (commit required), FWAPPS_NOT_CHANGED (ok)
} FWAppsEntry;

/* ---------------------------------------------------------------------------- */
/* Private functions */
gint              fwapps_get_checksum(gchar *path);
glong             fwapps_get_size(gchar *path);
glong             fwapps_get_mtime(gchar *path);

/* Public functions */

/* Add path to hash table, but application need commit */
gboolean          fwapps_add_path(gchar *path, glong mtime, glong size, gint chksum);

/* Add path to hash table and commit */
gboolean          fwapps_add_path_and_commit(gchar *path);

/* Commit path which has been uncommited or modified */
gboolean          fwapps_commit_path(gchar *path);

/* Remove path from application list */
gboolean          fwapps_remove_path(gchar *path);

/* Read application configuration from /etc/desktopfw/apps */
gint              fwapps_read(void);

/* Print configuration to out */
void              fwapps_print(FILE *out);

/* Write configuration to /etc/desktopfw/apps */
gboolean          fwapps_write(void);

/* Return fwapps status, FWAPPS_CHANGED means some things has 
   changed in application list - mtime, size or checksum */
FWAppsStatus      fwapps_check();

/* Return number of registered apps */
inline guint      fwapps_get_nr();

/* ---------------------------------------------------------------------------- */
gchar            *fwapps_cb_apps(gchar **strv);
gchar            *fwapps_cb_apps_check(gchar **strv);
gchar            *fwapps_cb_apps_status(gchar **strv);
gchar            *fwapps_cb_apps_addcommited(gchar **strv);
gchar            *fwapps_cb_apps_commit(gchar **strv);
gchar            *fwapps_cb_apps_remove(gchar **strv);
gchar            *fwapps_cb_apps_add(gchar **strv);
void              fwapps_register_cbs();

#endif 
