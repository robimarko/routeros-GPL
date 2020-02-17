/* x_tables module for setting the IPv4/IPv6 DSCP field
 *
 * (C) 2002 Harald Welte <laforge@gnumonks.org>
 * based on ipt_FTOS.c (C) 2000 by Matthew G. Marsh <mgm@paktronix.com>
 * This software is distributed under GNU GPL v2, 1991
 *
 * See RFC2474 for a description of the DSCP field within the IP Header.
 *
 * xt_DSCP.h,v 1.7 2002/03/14 12:03:13 laforge Exp
*/
#ifndef _XT_DSCP_TARGET_H
#define _XT_DSCP_TARGET_H
#include <linux/netfilter/xt_dscp.h>
#include <linux/types.h>

#define XT_DSCP_FROM_PRIORITY 255
#define XT_DSCP_HIGH_3_BITS_FROM_PRIORITY 254

/* target info */
struct xt_DSCP_info {
	__u8 dscp;
	__u8 passthrough;
};

struct xt_tos_target_info {
	__u8 tos_value;
	__u8 tos_mask;
};

#endif /* _XT_DSCP_TARGET_H */
