/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWNFQ_UDP_H
#define __FWNFQ_UDP_H

#include <fwcommon.h>

void              fwnfq_udpip_fprintf(FILE *out, struct ip *hdr, struct udphdr *udphdr, FWDirection dir, gint ifaceidx);
void              fwnfq_udphdr_fprintf(FILE *out, struct udphdr *udphdr, FWDirection dir, gint ifaceidx);

#endif 
