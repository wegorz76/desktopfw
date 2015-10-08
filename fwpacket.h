/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */

/*! \file fwpacket.h
  \brief \a FW packet header file.
*/

#if !defined __FWPACKET__H
#define __FWPACKET__H

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
typedef enum { 
  DIR_UNKNOWN,
  DIR_INCOMING,
  DIR_OUTGOING
} FWDirection;

typedef enum {
  PROTO_UNKNOWN = -1,
  PROTO_IP      = 0,
  PROTO_ICMP    = 1,
  PROTO_TCP     = 6,
  PROTO_UDP     = 17,
} FWProto;

typedef enum {
  ACTION_UNKNOWN,
  ACTION_ASK,
  ACTION_ACCEPT,
  ACTION_DROP,
  ACTION_REJECT
} FWAction;

extern    gchar *TxtDirection[];
extern    gchar *TxtProto[];
extern    gchar *TxtAction[];
extern    gchar *TxtTcpState[];
/* ---------------------------------------------------------------------------- */
FWDirection fwpacket_get_direction_by_name(gchar *direction);
FWProto     fwpacket_get_proto_by_name(gchar *proto);
FWAction    fwpacket_get_action_by_name(gchar *action);

#define IS_FWACTION(action) ( (action == ACTION_ASK) ||	    \
			      (action == ACTION_ACCEPT) ||  \
			      (action == ACTION_DROP) ||    \
			      (action == ACTION_REJECT ) )


#endif
