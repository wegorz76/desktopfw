/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */

/*! \file fwcommon.h
  \brief \a FW common header file.
*/

#if !defined __FWCOMMON__H
#define __FWCOMMON__H

/* Includes */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

/* BSD like structure fields naming */
#define __FAVOR_BSD

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

/* GLIB */
#include <glib.h>
#include <fwmacros.h>
#include <fwutils.h>
#include <fwpacket.h>
#include <fwconfig.h>
#include <fwcomm.h>
#include <fwrules.h>
#include <fwproc.h>
#include <fwdev.h>

/* ---------------------------------------------------------------------------- */
/*! \brief System environment variables used in \a FW. */
#define           FW_ENV_DEBUG_LEVEL                  "FW_DEBUG_LEVEL"
#define           FW_ENV_DEBUG_FUNCS_LEVEL            "FW_DEBUG_FUNCS_LEVEL"
#define           FW_ENV_DEBUG_FILEPOS_LEVEL          "FW_DEBUG_FILEPOS_LEVEL"

G_LOCK_EXTERN(FWGlobalLock);
#define           FW_GLOBAL_LOCK(...)   G_LOCK(FWGlobalLock)
#define           FW_GLOBAL_UNLOCK(...) G_UNLOCK(FWGlobalLock)

/* ---------------------------------------------------------------------------- */
/*! \brief Sections defined in configuration files */
typedef enum {
  SETTINGS = 1,
  APPS,
  RULES
} FWCfgSection;

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
void              fw_filepos_print(gchar *file, gint line, gchar *function); /*< \brief Function used by \a FW message macros. */
void              fw_init(gint argc, gchar **argv);                          /*!< \brief Initialization. */

void              fw_command_hash_init();
void              fw_command_add_callback(gchar *cmd, void *callback);
gchar            *fw_command_show_version(gchar **strv);
gchar            *fw_command_list(gchar **strv);
void              fw_command_print_registered_cmds();

gchar            *fw_command_call(gchar **strv);

//hostip - host ip in network byte order
gchar            *fw_nethostip_strnew(guint32 hostip);
gchar            *fw_nethostip_mask_strnew(guint32 hostip, gint mask);

gboolean          fw_get_hostip_mask(gchar *hoststr, guint32 *hostip, gint *mask);

#endif
