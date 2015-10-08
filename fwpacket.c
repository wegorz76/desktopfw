/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwpacket.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
gchar *TxtDirection[] = { "UNKNOWN", "incoming", "outgoing" };
gchar *TxtProto[]     = { "", "icmp", "", "", "", "", "tcp", "", "", "", "", "", "", "", "", "", "", "udp" };
gchar *TxtAction[]    = { "UNKNOWN", "ASK", "ACCEPT", "DROP", "REJECT" };

/* ---------------------------------------------------------------------------- */
gchar *TxtTcpState[] =
{
    "UNKNOWN",
    "ESTABLISHED",
    "SYN_SENT",
    "SYN_RECV",
    "FIN_WAIT1",
    "FIN_WAIT2",
    "TIME_WAIT",
    "CLOSE",
    "CLOSE_WAIT",
    "LAST_ACK",
    "LISTEN",
    "CLOSING"
};

/* ---------------------------------------------------------------------------- */
/*! Return Direction id by name. 
  \param action FWDirection string.
  \return FWDirection value.
*/

FWDirection fwpacket_get_direction_by_name(gchar *direction)
{
  FW_DEBUG_INFO3();

  if ( g_ascii_strcasecmp(direction, "incoming") == 0 )
    return DIR_INCOMING;
  else if ( g_ascii_strcasecmp(direction, "outgoing") == 0 )
    return DIR_OUTGOING;

  return DIR_UNKNOWN;
}

/* ---------------------------------------------------------------------------- */
/*! Return Protocol id by name. 
  \param proto FWProto string.
  \return FWProto value.
*/

FWProto     fwpacket_get_proto_by_name(gchar *proto)
{
  FW_DEBUG_INFO3();

  if ( g_ascii_strcasecmp(proto, "TCP") == 0 )
    return PROTO_TCP;
  else if ( g_ascii_strcasecmp(proto, "UDP") == 0 )
    return PROTO_UDP;
  else if ( g_ascii_strcasecmp(proto, "ICMP") == 0 )
    return PROTO_ICMP;
  else if ( g_ascii_strcasecmp(proto, "IP") == 0 )
    return PROTO_IP;

  return PROTO_UNKNOWN;
}

/* ---------------------------------------------------------------------------- */
/*! Return Action id by name. 
  \param action FWAction string.
  \return FWAction value.
*/
FWAction   fwpacket_get_action_by_name(gchar *action)
{
  FW_DEBUG_INFO3();

  if ( g_ascii_strcasecmp(action, "ASK") == 0 )
    return ACTION_ASK;
  else if ( g_ascii_strcasecmp(action, "ACCEPT") == 0 )
    return ACTION_ACCEPT;
  else if ( g_ascii_strcasecmp(action, "DROP") == 0 )
    return ACTION_DROP;
  else if ( g_ascii_strcasecmp(action, "REJECT") == 0 )
    return ACTION_REJECT;

  return ACTION_UNKNOWN;
}
