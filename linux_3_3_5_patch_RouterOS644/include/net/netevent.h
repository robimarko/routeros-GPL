#ifndef _NET_EVENT_H
#define _NET_EVENT_H

/*
 *	Generic netevent notifiers
 *
 *	Authors:
 *      Tom Tucker              <tom@opengridcomputing.com>
 *      Steve Wise              <swise@opengridcomputing.com>
 *
 * 	Changes:
 */

struct dst_entry;

struct netevent_redirect {
	struct dst_entry *old;
	struct dst_entry *new;
};

enum netevent_notif_type {
	NETEVENT_NEIGH_UPDATE = 1, /* arg is struct neighbour ptr */
	NETEVENT_REDIRECT,	   /* arg is struct netevent_redirect ptr */
#ifdef CONFIG_TILE_NETLIB
	NETEVENT_NEIGH_RELEASE, /* arg is struct neighbour release ptr */
	NETEVENT_FIB_INSERT,    /* arg is struct FIB insert ptr */
	NETEVENT_FIB_DELETE,    /* arg is struct FIB delete ptr */
#endif
};

extern int register_netevent_notifier(struct notifier_block *nb);
extern int unregister_netevent_notifier(struct notifier_block *nb);
extern int call_netevent_notifiers(unsigned long val, void *v);

#endif
