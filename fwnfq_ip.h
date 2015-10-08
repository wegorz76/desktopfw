/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWNFQ_IP_H
#define __FWNFQ_IP_H

#include <fwcommon.h>

void        fwnfq_iphdr_fprintf(FILE *out, struct ip *hdr, FWDirection dir, gint ifaceidx);

#endif 
