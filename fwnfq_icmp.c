/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwnfq_icmp.c
*/

#include <fwcommon.h>
#include <fwnfq_icmp.h>


gchar *TxtTypeICMP[] = {
  "Echo Reply", "", "", "Destination Unreachable", "Source Quench" "Redirect (change route)",
  "", "", "", "Echo Request", "", "", "Time Exceeded", "Parameter Problem", "Timestamp Request"
  "Timestamp Reply", "Information Request", "Information Reply", "Address Mask Request", "Address Mask Reply",
};

gchar *TxtUnreachICMP[] = {
  "Network Unreachable", "Host Unreachable", "Protocol Unreachable", "Port Unreachable",    
  "Fragmentation Needed/DF set", "Source Route failed", "Net unknown", "Host unknown",             
  "Host isolated", "Net ano", "Host ano", "Net unr TOS", "Host unr TOS", "Packet filtered",            
  "Precedence violation", "Precedence cut off",
};    

gchar *TxtRedirICMP[] = {
  "Redirect Net", "Redirect Host", "Redirect Net for TOS", "Redirect Host for TOS",
};

gchar *TxtTimeExcICMP[] = {
  "TTL count exceeded", "Fragment Reass time exceeded",
};

/* ---------------------------------------------------------------------------- */
void              fwnfq_icmphdr_fprintf(FILE *out, struct icmp *icmphdr, FWDirection dir, gint ifaceidx) 
{
  FWDevEntry *e = fwdev_entry_by_idx(ifaceidx);
  
  if (e == NULL) return;

  printf("icmp type: %d [%s]\n", icmphdr->icmp_type, TxtTypeICMP[icmphdr->icmp_type]);
  printf("icmp code: %d\n",      icmphdr->icmp_code);
  printf("icmp csum: %d\n",      ntohs(icmphdr->icmp_cksum));
}

/* ---------------------------------------------------------------------------- */
void              fwnfq_icmpip_fprintf(FILE *out, struct ip *hdr, struct icmp *icmphdr, FWDirection dir, gint ifaceidx) 
{
  FWDevEntry *e = fwdev_entry_by_idx(ifaceidx);
  gchar  *code = "";
  
  if (e == NULL) return;

  unsigned char *p = (unsigned char *) &hdr->ip_src;
  fprintf(out, "%d.%d.%d.%d -> ", p[0], p[1], p[2], p[3]);
  p = (unsigned char *) &hdr->ip_dst;
  fprintf(out, "%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);

  fprintf(out, "IP  [V: %d, HDRLEN: %d, TOS: %d, LEN: %d, ID: %d, TTL: %d, CSUM: %x]\n", 
	  hdr->ip_v, hdr->ip_hl, hdr->ip_tos, 
	  ntohs(hdr->ip_len), ntohs(hdr->ip_id), hdr->ip_ttl, ntohs(hdr->ip_sum));

  switch ( icmphdr->icmp_type )
    {
    case ICMP_DEST_UNREACH:
      code = TxtUnreachICMP[icmphdr->icmp_code];
      break;
      ;;
    case ICMP_REDIRECT:
      code = TxtRedirICMP[icmphdr->icmp_code];
      break;
      ;;
    case ICMP_TIME_EXCEEDED:
      code = TxtTimeExcICMP[icmphdr->icmp_code];
      break;
      ;;      
  }
  fprintf(out, "ICMP [TYPE: %d (%s), CODE: %d (%s), CSUM: %04x]\n\n", 
	  icmphdr->icmp_type, TxtTypeICMP[icmphdr->icmp_type],
	  icmphdr->icmp_code, code,
	  ntohs(icmphdr->icmp_cksum));
}

/*
void        fwnfq_print_icmphdr(void *data, gint ifi, gint ifo) 
{
  struct ip      *hdr;
  struct icmp    *icmphdr;

  hdr = (struct ip *) data;

  if (hdr->ip_p == PROTO_ICMP) {
    icmphdr = (struct icmp *) (data + 20);
    printf("*** ICMP ***\n");

    if (ifi) {
      gchar iface[17];
      if ( fwdev_find_iface(ifi, iface) )
	packet_action = fwconfig_entry_get_action(-2, iface, DIR_INCOMING, PROTO_ICMP, "0/0", ntohs(0), "*");
    }
    else if (ifo) {
      gchar iface[17];
      if ( fwdev_find_iface(ifo, iface) )
	packet_action = fwconfig_entry_get_action(-2, iface, DIR_OUTGOING, PROTO_ICMP, "0/0", ntohs(0), "*");
    }
  
  }
}
*/
