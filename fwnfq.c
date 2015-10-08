/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwnfq.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include <glib.h>
#include <fwmacros.h>
#include <fwcommon.h>
#include <fwconfig.h>
#include <fwdev.h>
#include <fwquery.h>
#include <fwnfq.h>
#include <fwnfq_ip.h>
#include <fwnfq_icmp.h>
#include <fwnfq_tcp.h>
#include <fwnfq_udp.h>

struct nfq_handle   *h;
struct nfq_q_handle *qh;
struct nfnl_handle  *nh;
gint                 fd;

/* ---------------------------------------------------------------------------- */
/* returns: packet id
   stores:  FWAction in *action 
*/
   
static u_int32_t fwnfq_process_pkt (struct nfq_data *tb, FWAction *action)
{
	struct nfqnl_msg_packet_hdr *ph;
	gint           id = 0;
	u_int32_t      mark, ifi, ifo, ifaceidx; 
	gint           ret, iphdrlen;
	gchar         *data;

	FWDirection     dir;
	struct ip      *hdr;
	
	ph = nfq_get_msg_packet_hdr(tb);
	if ( ph ) {
		id = ntohl(ph->packet_id);
		printf("hw_protocol=0x%04x hook=%u id=%u ", ntohs(ph->hw_protocol), ph->hook, id);
	}
	
	mark = nfq_get_nfmark(tb);
	if ( mark )
		printf("mark=%u ", mark);
	
	ifi = nfq_get_indev(tb);
	if ( ifi ) {
		printf("indev=%u ", ifi-1);
		dir   = DIR_INCOMING;
		ifaceidx = ifi - 1;
	}

	ifo = nfq_get_outdev(tb);
	if ( ifo ) {
		printf("outdev=%u ", ifo-1);
		dir   = DIR_OUTGOING;
		ifaceidx = ifo - 1;
	}

	ret = nfq_get_payload(tb, &data);
	if ( ret >= 0 )
		printf("payload_len=%d ", ret);

	printf("\n\n");

	/* Refresh every 50 packet */
	//  if ( (id % 50) == 0 ) {
	//    fwproc_netentries_process(PROTO_TCP);
	//    fwproc_netentries_process(PROTO_UDP);
	//    fwproc_scan_procdir();
	//  }

	hdr      = (struct ip *) data;
	iphdrlen = hdr->ip_hl * 4; // IP header len = ip_hl * 4 (32 bits)
	printf("*** PACKET PROTOCOL: %s ***\n", TxtProto[hdr->ip_p]);
	
	*action = fwnfq_process_rules(ifaceidx, dir, data, id);

	return id;
}
	

/* ---------------------------------------------------------------------------- */
static gint       fwnfq_callback(struct nfq_q_handle *qh, 
				  struct nfgenmsg     *nfmsg, 
				  struct nfq_data     *nfa, 
				  void *data)
{
	FWAction action;
	u_int32_t id = fwnfq_process_pkt(nfa, &action);

	printf("Entering callback [%d]\n", id);
	printf("Packet action: [%d]\n", action);
	
	if (action == ACTION_ACCEPT) {
		printf("ACTION: * ACCEPT * \n\n");
		return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
	} 
	else if (action == ACTION_DROP) {
	  printf("ACTION: * DROP * \n\n");
	  return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
	}
	else if (action == ACTION_REJECT) {
		printf("ACTION: * REJECT (now DROP) * \n\n");
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
	}
	else {    
		printf("ACTION: * DEFAULT: ASK (packet QUEUED) * \n\n");
		return 0;
		//    return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);	  
	}
}

/* ---------------------------------------------------------------------------- */
/* nfq initialize
 */
void              fwnfq_init(void)
{
	g_message("Opening library handle");
	h = nfq_open();
	if (!h)
		g_error("Error during nfq_open()");
	
	g_message("unbinding existing nf_queue handler for AF_INET (if any)");
	if (nfq_unbind_pf(h, AF_INET) < 0)
		g_warning("Error during nfq_unbind_pf()");
	
	g_message("Binding nfnetlink_queue as nf_queue handler for AF_INET");
	if (nfq_bind_pf(h, AF_INET) < 0)
		g_error("Error during nfq_bind_pf()");
	
	g_message("Binding this socket to queue '0'");
	qh = nfq_create_queue(h,  0, &fwnfq_callback, NULL);
	if (!qh) 
		g_error("Error during nfq_create_queue()\n");

	g_message("Setting copy_packet mode");
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0)
		g_error("Can't set packet_copy mode");
  
	nh = nfq_nfnlh(h);
	fd = nfnl_fd(nh);
}

/* ---------------------------------------------------------------------------- */
void              fwnfq_destroy(void)
{  
	g_message("Unbinding from queue 0");
	nfq_destroy_queue(qh);
  
#ifdef INSANE
	/* normally, applications SHOULD NOT issue this command, since
	 * it detaches other programs/sockets from AF_INET, too ! */
	g_message("Unbinding from AF_INET");
	nfq_unbind_pf(h, AF_INET);
#endif

	g_message("Closing library handle");
	nfq_close(h);
}

/* ---------------------------------------------------------------------------- */
void              fwnfq_loop(void)
{
	gchar buf[4096];
	gint  rv;
	while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
		printf("=====================================================================\n");
		printf("* pkt received *\n");
		nfq_handle_packet(h, buf, rv);
	}
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
FWAction          fwnfq_process_rules(gint ifaceidx, FWDirection dir, gchar *data, guint32 id)
{
	FWAction        action;
	gint            iphdrlen;
	struct ip      *hdr;
	struct icmp    *icmphdr;
	struct tcphdr  *tcphdr;
	struct udphdr  *udphdr;
	gchar          *application = NULL;
	guint32         srcip;
	guint32         dstip;
	gint            sport;
	gint            dport;
	gboolean        packet_established = FALSE;

	fwdev_read(); //keep a devices list up to date

	hdr      = (struct ip *) data;
	iphdrlen = hdr->ip_hl * 4; // IP header len = ip_hl * 4 (32 bits)

	srcip =  ntohl( *((guint32 *) &hdr->ip_src) );
	dstip =  ntohl( *((guint32 *) &hdr->ip_dst) );
	printf("srcip = [%x]\n", srcip);
	printf("dstip = [%x]\n", dstip);

	/* ICMP packet */
	if ( hdr->ip_p == PROTO_ICMP ) {
		icmphdr = (struct icmp *) (data + iphdrlen);
		fwnfq_icmpip_fprintf(stdout, hdr, icmphdr, dir, ifaceidx);
	}

	/* TCP packet */
	else if ( hdr->ip_p == PROTO_TCP )
    {      
		tcphdr = (struct tcphdr *) (data + iphdrlen);
		sport =  ntohs(tcphdr->th_sport);
		dport =  ntohs(tcphdr->th_dport);
		fwnfq_tcpip_fprintf(stdout, hdr, tcphdr, dir, ifaceidx);
		if ( tcphdr->th_flags & TH_ACK || tcphdr->th_flags & TH_RST ) {
			packet_established = TRUE;
			application = g_strdup("*");
		} else {
			packet_established = FALSE;
			fwproc_rescan_all();
			fwproc_netentries_fprintf(stdout, PROTO_TCP, TCP_SYN_SENT);
			if (dir == DIR_OUTGOING) 
				application = fwproc_netentries_tcpapp_getnew(srcip, dstip, sport, dport);
			else 
				application = fwproc_netentries_tcpapp_getnew(dstip, srcip, dport, sport);
			application = fwproc_netentries_tcpapp_getnew(srcip, dstip, sport, dport);
			printf("Aplication: %s\n", application);
		}
    }
  
	/* UDP packet */
	else if (hdr->ip_p == PROTO_UDP) {
		udphdr = (struct udphdr *) (data + iphdrlen);
    
		fwnfq_udpip_fprintf(stdout, hdr, udphdr, dir, ifaceidx);    
	}

	printf("Process Aplication: %s\n", application);

	if (packet_established == FALSE) {
		fwrules_remove_timeouted();
		action = fwrules_entry_get_action(ifaceidx, dir, hdr->ip_p, srcip, sport, dstip, dport, application);
	}
	else 
		action = ACTION_ACCEPT;

	if (action == ACTION_ASK && packet_established == FALSE) {
		printf("Rule not found. ASK user\n");

		gchar *ifacestr = fwdev_string_by_nf_idx(ifaceidx);
		if (!ifacestr)
			g_assert("Bug!");
		
		fwquery_add( ifacestr, dir, hdr->ip_p,
					 application, srcip, dstip, sport, dport, id);
	}

	FW_FREEN(application);

	return action;
}

/* ---------------------------------------------------------------------------- */
gint              fwnfq_accept_packet(guint32 id) {
	return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

gint              fwnfq_drop_packet(guint32 id) {
	return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
}
