/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwnfq_tcp.c
*/

#include <fwcommon.h>
#include <fwnfq_tcp.h>

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/*! Return a new allocated TCP flags info string.
  \param flags TCP flags.
  \return New allocated string which contains info about TCP flags.
  \note String need to be freed after usage.
*/
gchar      *fwnfq_tcpflags_to_new_string(u_int8_t flags)
{
  gint len;
  gchar *str = g_new0(gchar, FLAGS_BUFSIZE);

  str = (flags & TH_FIN)  ? strncat(str, "FIN * ", FLAGS_BUFSIZE-1) : str;
  str = (flags & TH_SYN)  ? strncat(str, "SYN * ", FLAGS_BUFSIZE-1) : str;
  str = (flags & TH_RST)  ? strncat(str, "RST * ", FLAGS_BUFSIZE-1) : str;
  str = (flags & TH_PUSH) ? strncat(str, "PSH * ", FLAGS_BUFSIZE-1) : str;
  str = (flags & TH_ACK)  ? strncat(str, "ACK * ", FLAGS_BUFSIZE-1) : str;
  str = (flags & TH_URG)  ? strncat(str, "URG * ", FLAGS_BUFSIZE-1) : str;

  if ( (len = strlen(str)) ) str[len-3] = 0;

  return str;
}

/* ---------------------------------------------------------------------------- */
void        fwnfq_tcphdr_fprintf(FILE *out, struct tcphdr *tcphdr, FWDirection dir, gint ifaceidx) 
{
  FWDevEntry *e = fwdev_entry_by_idx(ifaceidx);
  gchar  *bufflags;
  
  if (e == NULL) return;

  //  fprintf(out, "*** TCP ***\n");
  //  fprintf(out, "*** Interface [%s], Direction: %s***\n", strif, TxtDirection[dir]);

  bufflags = fwnfq_tcpflags_to_new_string(tcphdr->th_flags);

  fprintf(out, "srcport:    %d\n", ntohs(tcphdr->th_sport));
  fprintf(out, "dstport:    %d\n", ntohs(tcphdr->th_dport));
  fprintf(out, "seq:        %x\n", ntohl(tcphdr->th_seq));
  fprintf(out, "ack:        %x\n", ntohl(tcphdr->th_ack));
  fprintf(out, "tcplen:     %d\n", tcphdr->th_x2 >> 2);
  fprintf(out, "flags:      %d [%s]\n", tcphdr->th_flags, bufflags);   
  fprintf(out, "win:        %x\n", ntohs(tcphdr->th_win));
  fprintf(out, "sum:        %x\n", ntohs(tcphdr->th_sum));
  fprintf(out, "urp:        %x\n", ntohs(tcphdr->th_urp));
  g_free(bufflags);  
}

/* ---------------------------------------------------------------------------- */
void        fwnfq_tcpip_fprintf(FILE *out, struct ip *hdr, struct tcphdr *tcphdr, FWDirection dir, gint ifaceidx) 
{
  FWDevEntry *e = fwdev_entry_by_idx(ifaceidx);
  gchar  *bufflags;
  
  if (e == NULL) return;

  guchar *p = (guchar *) &hdr->ip_src;
  fprintf(out, "%d.%d.%d.%d:%d -> ", p[0], p[1], p[2], p[3], ntohs(tcphdr->th_sport));
  p = (unsigned char *) &hdr->ip_dst;
  fprintf(out, "%d.%d.%d.%d:%d\n", p[0], p[1], p[2], p[3], ntohs(tcphdr->th_dport));

  fprintf(out, "IP  [V: %d, HDRLEN: %d, TOS: %d, LEN: %d, ID: %d, TTL: %d, CSUM: %x]\n", 
	  hdr->ip_v, hdr->ip_hl, hdr->ip_tos, 
	  ntohs(hdr->ip_len), ntohs(hdr->ip_id), hdr->ip_ttl, ntohs(hdr->ip_sum));


  bufflags = fwnfq_tcpflags_to_new_string(tcphdr->th_flags);
  
  fprintf(out, "TCP [SEQ: %08x, ACK: %08x, TCPLEN: %d, WIN: %04x, SUM: %04x, URP: %x, FLAGS: %s]\n\n", 
	  ntohl(tcphdr->th_seq), ntohl(tcphdr->th_ack), tcphdr->th_x2 >> 2,
	  ntohs(tcphdr->th_win), ntohs(tcphdr->th_sum), ntohs(tcphdr->th_urp), bufflags);

  g_free(bufflags);  
}

/* ---------------------------------------------------------------------------- */
/*
void        fwnfq_print_tcphdr(void *data, gint ifi, gint ifo) 
{
  struct ip     *hdr;
  struct tcphdr *tcphdr;
  gchar  *bufflags;

  hdr = (struct ip *) data;

  if (hdr->ip_p == 6) {
    tcphdr = (struct tcphdr *) (data + 20);
    printf("*** TCP ***\n");

    if (ifi) {
      gchar iface[17];
      if ( fwdev_find_iface(ifi, iface) )
	packet_action = fwconfig_entry_get_action(-2, iface, DIR_INCOMING, PROTO_TCP, "0/0", ntohs(tcphdr->th_sport), "*");
    }
    else if (ifo) {
      gchar iface[17];
      if ( fwdev_find_iface(ifo, iface) )
	packet_action = fwconfig_entry_get_action(-2, iface, DIR_OUTGOING, PROTO_TCP, "0/0", ntohs(tcphdr->th_dport), "*");
    }
    printf("srcport:    %d\n", ntohs(tcphdr->th_sport));
    printf("dstport:    %d\n", ntohs(tcphdr->th_dport));
    printf("seq:        %x\n", ntohl(tcphdr->th_seq));
    printf("ack:        %x\n", ntohl(tcphdr->th_ack));
    printf("tcplen:     %d\n", tcphdr->th_x2 >> 2);
    printf("flags:      %d\n", tcphdr->th_flags);   
    printf("win:        %x\n", ntohs(tcphdr->th_win));
    printf("sum:        %x\n", ntohs(tcphdr->th_sum));
    printf("urp:        %x\n", ntohs(tcphdr->th_urp));

    bufflags = fwnfq_tcpflags_to_new_string(tcphdr->th_flags);
    printf("tcpflags:   %s\n",    bufflags);
    g_free(bufflags);
  }
}
*/
