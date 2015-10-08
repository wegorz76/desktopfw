/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWCOMM_H
#define __FWCOMM_H

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */

/* Create communication thread */
gboolean          fwcomm_thread_create();
inline gboolean   fwcomm_thread_is_running();

gchar            *fwcomm_cb_auth(gchar **);
void              fwcomm_register_cbs();

#endif 
