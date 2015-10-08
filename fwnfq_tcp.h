/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWNFQ_TCP_H
#define __FWNFQ_TCP_H

#include <fwcommon.h>

#define FLAGS_BUFSIZE 39

gchar      *fwnfq_tcpflags_to_new_string(u_int8_t flags);
void        fwnfq_tcpip_fprintf(FILE *out, struct ip *hdr, struct tcphdr *tcphdr, FWDirection dir, gint ifaceidx);
void        fwnfq_tcphdr_fprintf(FILE *out, struct tcphdr *tcphdr, FWDirection dir, gint ifaceidx);

#endif 
