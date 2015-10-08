/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWNFQ_ICMP_H
#define __FWNFQ_ICMP_H

#include <fwcommon.h>

void        fwnfq_icmphdr_fprintf(FILE *out, struct icmp *icmphdr, FWDirection dir, gint ifaceidx);
void        fwnfq_icmpip_fprintf(FILE *out, struct ip *hdr, struct icmp *icmphdr, FWDirection dir, gint ifaceidx);

#endif 
