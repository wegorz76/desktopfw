/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwnfq_udp.c
*/

#include <fwcommon.h>
#include <fwnfq_udp.h>

/* ---------------------------------------------------------------------------- */
void        fwnfq_udphdr_fprintf(FILE *out, struct udphdr *udphdr, FWDirection dir, gint ifaceidx) 
{
  FWDevEntry *e = fwdev_entry_by_idx(ifaceidx);
  
  if (e == NULL) return;

  fprintf(out, "srcport:    %d\n", ntohs(udphdr->uh_sport));
  fprintf(out, "dstport:    %d\n", ntohs(udphdr->uh_dport));
  fprintf(out, "length:     %d\n", ntohs(udphdr->uh_ulen));
  fprintf(out, "chksum:     %x\n", ntohs(udphdr->uh_sum));
}

/* ---------------------------------------------------------------------------- */
void        fwnfq_udpip_fprintf(FILE *out, struct ip *hdr, struct udphdr *udphdr, FWDirection dir, gint ifaceidx) 
{
  FWDevEntry *e = fwdev_entry_by_idx(ifaceidx);
  
  if (e == NULL) return;

  unsigned char *p = (unsigned char *) &hdr->ip_src;
  fprintf(out, "%d.%d.%d.%d:%d -> ", p[0], p[1], p[2], p[3], ntohs(udphdr->uh_sport));
  p = (unsigned char *) &hdr->ip_dst;
  fprintf(out, "%d.%d.%d.%d:%d\n", p[0], p[1], p[2], p[3], ntohs(udphdr->uh_dport));

  fprintf(out, "IP  [V: %d, HDRLEN: %d, TOS: %d, LEN: %d, ID: %d, TTL: %d, CSUM: %x]\n", 
	  hdr->ip_v, hdr->ip_hl, hdr->ip_tos, 
	  ntohs(hdr->ip_len), ntohs(hdr->ip_id), hdr->ip_ttl, ntohs(hdr->ip_sum));

  fprintf(out, "UDP [LEN: %d, SUM: %04x]\n\n", ntohs(udphdr->uh_ulen), ntohs(udphdr->uh_sum));
}

 /*
void        fwnfq_print_udphdr(void *data, gint ifi, gint ifo) 
{
  struct ip     *hdr;
  struct udphdr *udphdr;

  hdr = (struct ip *) data;

  if (hdr->ip_p == 17) {
    udphdr = (struct udphdr *) (data + 20);
    printf("*** UDP ***\n");
    printf("srcport:    %d\n",     ntohs(udphdr->uh_sport));
    printf("dstport:    %d\n",     ntohs(udphdr->uh_dport));
    printf("length:     %d\n",     ntohs(udphdr->uh_ulen));
    printf("chksum:     %x\n",     ntohs(udphdr->uh_sum));

    if (ifi) {
      gchar iface[17];
      if ( fwdev_find_iface(ifi, iface) )
	packet_action = fwconfig_entry_get_action(-2, iface, DIR_INCOMING, PROTO_UDP, "0/0", ntohs(udphdr->uh_sport), "*");
    }
    else if (ifo) {
      gchar iface[17];
      if ( fwdev_find_iface(ifo, iface) )
	packet_action = fwconfig_entry_get_action(-2, iface, DIR_OUTGOING, PROTO_UDP, "0/0", ntohs(udphdr->uh_dport), "*");
    }
  }
}
 */
