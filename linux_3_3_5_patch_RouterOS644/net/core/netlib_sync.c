/*
 *	Sync route table&neighbor table from kernel to user space through
 *  netlink.
 *
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 */
#ifdef CONFIG_TILE_NETLIB

#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/netdevice.h>
#include <linux/times.h>
#include <net/net_namespace.h>
#include <net/neighbour.h>
#include <net/dst.h>
#include <net/sock.h>
#include <net/netevent.h>
#include <net/netlink.h>
#include <net/ip_fib.h>
#include <linux/rtnetlink.h>
#include <linux/random.h>
#include <linux/string.h>

/* #define NETLIB_SYNC_DEBUG 1 */

struct nl_neighbour_msg {
	uint32_t		event; /* Neighbour update or release */
	uint8_t			flags;
	uint8_t			nud_state;
	uint8_t			type;
	uint8_t			pad;
	char			name[IFNAMSIZ];
	unsigned char		ha[ALIGN(MAX_ADDR_LEN, sizeof(unsigned long))];
	u8			primary_key[4];
};
struct nl_fib_msg {
	uint32_t		event;
	uint8_t			rt_family;
	uint8_t			rt_dst_len;
	uint8_t			rt_src_len;
	uint8_t			rt_tos;
	uint8_t			rt_protocol;
	uint8_t			rt_scope;
	uint8_t			rt_type;
	uint8_t			rt_nmetrics;
	char			rt_oif_name[IFNAMSIZ];
	uint32_t		rt_flags;
	uint32_t		rt_dst;
	uint32_t		rt_src;
	uint32_t		rt_table;
	uint32_t		rt_oif;
	uint32_t		rt_prio;
	uint32_t		rt_metrics_mask;
	uint32_t		rt_nr_nh;
	uint32_t		rt_gw;
};
struct sock *nl_sync_sk;

static int netlib_sync_fib(unsigned long event, void *ptr)
{
	struct nl_fib_msg fib_msg;
	struct nl_fib_msg *nl_fib_msg;
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	struct fib_config *cfg;
#ifdef NETLIB_SYNC_DEBUG
	unsigned int addr;
#endif
	struct net *net;
	struct net_device *dev;
	int err;

	cfg = (struct fib_config *)ptr;
	net = cfg->fc_nlinfo.nl_net;
	memset(&fib_msg, 0, sizeof(fib_msg));
	fib_msg.rt_dst_len = cfg->fc_dst_len;
	fib_msg.rt_dst = cfg->fc_dst;
	fib_msg.rt_tos = cfg->fc_tos;
	fib_msg.rt_protocol = cfg->fc_protocol;
	fib_msg.rt_scope = cfg->fc_scope;
	fib_msg.rt_type = cfg->fc_type;
	fib_msg.rt_flags = cfg->fc_flags;
	fib_msg.rt_src = cfg->fc_prefsrc;
	fib_msg.rt_table = cfg->fc_table;
	fib_msg.rt_oif = cfg->fc_oif;
	fib_msg.rt_prio = cfg->fc_priority;
	fib_msg.rt_gw = cfg->fc_gw;
	dev = dev_get_by_index(net, cfg->fc_oif);
	memcpy(fib_msg.rt_oif_name, dev->name, IFNAMSIZ);
	fib_msg.event = event;

	switch (event) {
	case NETEVENT_FIB_INSERT:
		break;

	case NETEVENT_FIB_DELETE:
		break;

	default:
		return -EINVAL;
	}

	#ifdef NETLIB_SYNC_DEBUG
	addr = fib_msg.rt_dst;
	pr_info("route update:%d\n", event);
	pr_info("%s family: %d dst:%d.%d.%d.%d dst_mask:%d oif:%d",
		fib_msg.rt_oif_name, fib_msg.rt_family,
		((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1],
		((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3],
		fib_msg.rt_dst_len, fib_msg.rt_oif);
	pr_info("gw:%d.%d.%d.%d tid: %d\n",
		((unsigned char *)&fib_msg.rt_gw)[0],
		((unsigned char *)&fib_msg.rt_gw)[1],
		((unsigned char *)&fib_msg.rt_gw)[2],
		((unsigned char *)&fib_msg.rt_gw)[3],
		fib_msg.rt_table);
	#endif

	skb = nlmsg_new(sizeof(struct nl_fib_msg), GFP_ATOMIC);
	if (skb != NULL) {
		nlh = nlmsg_put(skb, 0, 0, 0, sizeof(struct nl_fib_msg), 0);
		nl_fib_msg = (struct nl_fib_msg *)nlmsg_data(nlh);
		memcpy(nl_fib_msg, &fib_msg, sizeof(struct nl_fib_msg));
		nlh->nlmsg_pid = 0;
		nlh->nlmsg_flags = 0;
		NETLINK_CB(skb).pid = 0;
		NETLINK_CB(skb).dst_group = 1;
		err = nlmsg_multicast(nl_sync_sk, skb, 0, 1, GFP_ATOMIC);
		if (err < 0)
			return -1;
	} else
		panic("fib_netlink_event: alloc skb failed!\n");

	return 0;
}

static int netlib_sync_neighbor(unsigned long event, void *ptr)
{
	struct nl_neighbour_msg nl_neigh;
	struct neighbour *neigh = NULL;
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	struct nl_neighbour_msg *nl_msg;
	int err;

	neigh = (struct neighbour *)ptr;

	if (neigh->nud_state == NUD_NOARP)
		return -NUD_NOARP;

	switch (event) {
	case NETEVENT_NEIGH_UPDATE:
		if (neigh->nud_state == NUD_STALE)
			return -NUD_STALE;
		break;

	case NETEVENT_NEIGH_RELEASE:
		break;

	default:
		return -EINVAL;
	}

	memset(&nl_neigh, 0, sizeof(struct nl_neighbour_msg));
	nl_neigh.flags = neigh->flags;
	nl_neigh.nud_state = neigh->nud_state;
	nl_neigh.type = neigh->type;
	memcpy(nl_neigh.ha, neigh->ha, ALIGN(MAX_ADDR_LEN,
		sizeof(unsigned long)));
	memcpy(nl_neigh.name, neigh->dev->name, IFNAMSIZ);
	memcpy(nl_neigh.primary_key, neigh->primary_key, 4);
	nl_neigh.event = event;

	#ifdef NETLIB_SYNC_DEBUG
	pr_info("Neigh update(%d): %02x:%02x:%02x:%02x:%02x:%02x,%d-%d-%d-%d",
		event, neigh->ha[0], neigh->ha[1], neigh->ha[2], neigh->ha[3],
		neigh->ha[4], neigh->ha[5], neigh->primary_key[0],
		neigh->primary_key[1], neigh->primary_key[2],
		neigh->primary_key[3]);
	pr_info(" - f:%x t:%x n:%x\n", neigh->flags, neigh->type,
		neigh->nud_state);
	#endif

	skb = nlmsg_new(sizeof(struct nl_neighbour_msg), GFP_ATOMIC);
	if (skb != NULL) {
		nlh = nlmsg_put(skb, 0, 0, 0,
				sizeof(struct nl_neighbour_msg), 0);
		nl_msg = (struct nl_neighbour_msg *)nlmsg_data(nlh);
		memcpy(nl_msg, &nl_neigh, sizeof(struct nl_neighbour_msg));
		nlh->nlmsg_pid = 0;
		nlh->nlmsg_flags = 0;
		NETLINK_CB(skb).pid = 0;
		NETLINK_CB(skb).dst_group = 1;
		err = nlmsg_multicast(nl_sync_sk, skb, 0, 1, GFP_ATOMIC);
		if (err < 0)
			return -1;
	} else
		panic("netlib_sync_event: alloc skb failed!\n");

	return 0;
}

static int netlib_sync_event(struct notifier_block *this, unsigned long event,
				void *ptr)
{
	int err;

	switch (event) {
	case NETEVENT_NEIGH_UPDATE:
	case NETEVENT_NEIGH_RELEASE:
		err = netlib_sync_neighbor(event, ptr);
		break;
	case NETEVENT_FIB_INSERT:
	case NETEVENT_FIB_DELETE:
		err = netlib_sync_fib(event, ptr);
		break;
	default:
		err = -EINVAL;
		break;
	}

	return err;
}

static struct notifier_block netlib_sync_notifier = {
	.notifier_call = netlib_sync_event,
};

static void __exit netlib_sync_exit(void)
{
	netlink_kernel_release(nl_sync_sk);
	nl_sync_sk = NULL;
	unregister_netevent_notifier(&netlib_sync_notifier);
}


static int __init netlib_sync_init(void)
{
	pr_info("Load netlib_sync module\n");
	nl_sync_sk = netlink_kernel_create(&init_net, NETLINK_NETLIB_SYNC,
			1, NULL, NULL, THIS_MODULE);
	if (!nl_sync_sk) {
		pr_err("can not create NETLINK_NETLIB_SYNC\n");
		return -ENOMEM;
	}

	register_netevent_notifier(&netlib_sync_notifier);
	pr_info("netlib_sync: create netlink and register netevent\n");

	return 0;
}

module_init(netlib_sync_init);
module_exit(netlib_sync_exit);
#endif
