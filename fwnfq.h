/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWNFQ_H
#define __FWNFQ_H

#include <fwcommon.h>

void              fwnfq_init(void);
void              fwnfq_destroy(void);
void              fwnfq_loop(void);

FWAction          fwnfq_process_rules(gint ifaceidx, FWDirection dir, gchar *data, guint32 id);

//Functions needed to run by external part (fwqueue for ex)
gint              fwnfq_accept_packet(guint32 id);
gint              fwnfq_drop_packet(guint32 id);
#endif 
