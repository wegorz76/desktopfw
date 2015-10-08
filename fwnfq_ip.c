/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwnfq_ip.c
*/

#include <fwcommon.h>
#include <fwnfq_ip.h>

/* ---------------------------------------------------------------------------- */
void        fwnfq_iphdr_fprintf(FILE *out, struct ip *hdr, FWDirection dir, gint ifaceidx)
{
  FWDevEntry *e = fwdev_entry_by_idx(ifaceidx);
  
  if (e == NULL) return;

  printf("---------------------------------\n");
  printf("*** IP ***\n");
  printf("IP on iface: %s\n", e->iface);
  printf("IP Version:  %d\n", hdr->ip_v);
  printf("IP HdrLen:   %d (%d bytes)\n", hdr->ip_hl, hdr->ip_hl << 2);
  printf("IP TOS:      %d\n", hdr->ip_tos);
  printf("IP Len:      %d\n", ntohs(hdr->ip_len));
  printf("IP Id:       %d\n", ntohs(hdr->ip_id));
  printf("IP Off:      %d\n", ntohs(hdr->ip_off));
  printf("IP TTL:      %d\n", hdr->ip_ttl);
  printf("IP Proto:    %d\n", hdr->ip_p);
  printf("IP CSUM:     %d\n", ntohs(hdr->ip_sum));
  unsigned char *p = (unsigned char *) &hdr->ip_src;
  printf("IP srcaddr:  %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
  p = (unsigned char *) &hdr->ip_dst;
  printf("IP dstaddr:  %d.%d.%d.%d\n\n", p[0], p[1], p[2], p[3]);
}

