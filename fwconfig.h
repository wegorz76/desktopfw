/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWCONFIG_H
#define __FWCONFIG_H

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
#define FWCONFIG_BUFSIZE 17
#define FWCONFIG    "/etc/desktopfw/config"
#define FWCONFIGOLD "/etc/desktopfw/config.old"
#define BUFSIZE 1024

/* ---------------------------------------------------------------------------- */

gint              fwconfig_read(void);
void              fwconfig_print(FILE *out);
gboolean          fwconfig_write(void);

inline gchar     *fwconfig_get_fw_username();
inline gchar     *fwconfig_get_fw_password();
inline gchar     *fwconfig_get_fw_host();
inline gint       fwconfig_get_fw_port();
inline gint       fwconfig_get_fw_default_in_action();
inline gint       fwconfig_get_fw_default_out_action();

gboolean          fwconfig_set_fw_username(gchar *username);
gboolean          fwconfig_set_fw_password(gchar *password);
gboolean          fwconfig_set_fw_host(gchar *host);
gboolean          fwconfig_set_fw_port(gint port);
gboolean          fwconfig_set_fw_default_in_action(FWAction action);
gboolean          fwconfig_set_fw_default_out_action(FWAction action);

gchar            *fwconfig_cb_config_list(gchar **strv);
gchar            *fwconfig_cb_config_set(gchar **strv);
void              fwconfig_register_cbs();

#endif 
