/*
 * Generic PPP layer for Linux.
 *
 * Copyright 1999-2002 Paul Mackerras.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * The generic PPP layer handles the PPP network interfaces, the
 * /dev/ppp device, packet and VJ compression, and multilink.
 * It talks to PPP `channels' via the interface defined in
 * include/linux/ppp_channel.h.  Channels provide the basic means for
 * sending and receiving PPP frames on some kind of communications
 * channel.
 *
 * Part of the code in this driver was inspired by the old async-only
 * PPP driver, written by Michael Callahan and Al Longyear, and
 * subsequently hacked by Paul Mackerras.
 *
 * ==FILEVERSION 20041108==
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/netdevice.h>
#include <linux/poll.h>
#include <linux/ppp_defs.h>
#include <linux/filter.h>
#include <linux/if_ppp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp-comp.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <linux/stddef.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/etherdevice.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <linux/atomic.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#include <linux/nsproxy.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>

#define PPP_VERSION	"2.4.2"

#define MPHDRLEN	6	/* multilink protocol header length */
#define MPHDRLEN_SSN	4	/* ditto with short sequence numbers */

#define BCP_HDRLEN	2
#define BCP_LAN_FCS	0x80

/*
 * Bits in flags: SC_NO_TCP_CCID, SC_CCP_OPEN, SC_CCP_UP, SC_LOOP_TRAFFIC,
 * SC_MULTILINK, SC_MP_SHORTSEQ, SC_MP_XSHORTSEQ, SC_COMP_TCP, SC_REJ_COMP_TCP,
 * SC_MUST_COMP
 * Bits in rstate: SC_DECOMP_RUN, SC_DC_ERROR, SC_DC_FERROR.
 * Bits in xstate: SC_COMP_RUN
 */
#define SC_FLAG_BITS	(SC_NO_TCP_CCID|SC_CCP_OPEN|SC_CCP_UP|SC_LOOP_TRAFFIC \
			 |SC_MULTILINK|SC_MP_SHORTSEQ|SC_MP_XSHORTSEQ \
			 |SC_COMP_TCP|SC_REJ_COMP_TCP|SC_MUST_COMP|SC_CHANGE_MSS)

/*
 * SMP locking issues:
 * Both the ppp.rlock and ppp.wlock locks protect the ppp.channels
 * list and the ppp.n_channels field, you need to take both locks
 * before you modify them.
 * The lock ordering is: channel.upl -> ppp.wlock -> ppp.rlock ->
 * channel.downl.
 */

static DEFINE_MUTEX(ppp_mutex);
static atomic_t ppp_unit_count = ATOMIC_INIT(0);
static atomic_t channel_count = ATOMIC_INIT(0);

/* per-net private data for this module */
static int ppp_net_id __read_mostly;
struct ppp_net {
	/* units to ppp mapping */
	struct idr units_idr;

	/*
	 * all_ppp_mutex protects the units_idr mapping.
	 * It also ensures that finding a ppp unit in the units_idr
	 * map and updating its file.refcnt field is atomic.
	 */
	struct mutex all_ppp_mutex;

	/* channels */
	struct list_head all_channels;
	struct list_head new_channels;
	int last_channel_index;

	/*
	 * all_channels_lock protects all_channels and
	 * last_channel_index, and the atomicity of find
	 * a channel and updating its file.refcnt field.
	 */
	spinlock_t all_channels_lock;

	unsigned last_unit_index;

	struct list_head unreg_list;
	struct delayed_work unreg_work;
};

/* Get the PPP protocol number from a skb */
#define PPP_PROTO(skb)	get_unaligned_be16((skb)->data)

/* We limit the length of ppp->file.rq to this (arbitrary) value */
#define PPP_MAX_RQLEN	32

/*
 * Maximum number of multilink fragments queued up.
 * This has to be large enough to cope with the maximum latency of
 * the slowest channel relative to the others.  Strictly it should
 * depend on the number of channels and their characteristics.
 */
#define PPP_MP_MAX_QLEN	128

/* Multilink header bits. */
#define B	0x80		/* this fragment begins a packet */
#define E	0x40		/* this fragment ends a packet */

/* Compare multilink sequence numbers (assumed to be at least 12 bits wide) */
#define seq_before(a, b)	(((b - a) & 0xfff) < (0x1000 >> 1))

/* Prototypes. */
static int ppp_unattached_ioctl(struct net *net, struct ppp_file *pf,
			struct file *file, unsigned int cmd, unsigned long arg);
static void ppp_xmit_process(struct ppp *ppp, struct sk_buff *skb);
static struct sk_buff *pad_compress_skb(struct ppp *ppp, struct sk_buff *skb);
static int ppp_push(struct ppp *ppp, struct sk_buff *skb);
static void ppp_channel_push(struct channel *pch);
static void ppp_receive_frame(struct ppp *ppp, struct sk_buff *skb,
			      struct channel *pch);
static void ppp_receive_nonmp_frame(struct ppp *ppp, struct sk_buff *skb);
static struct sk_buff *ppp_decompress_frame(struct ppp *ppp,
					    struct sk_buff *skb);
#ifdef CONFIG_PPP_MULTILINK
static void ppp_receive_mp_frame(struct ppp *ppp, struct sk_buff *skb,
				struct channel *pch);
static struct sk_buff *ppp_mp_reconstruct(struct ppp *ppp);
static int ppp_mp_explode(struct ppp *ppp, struct sk_buff *skb);
#endif /* CONFIG_PPP_MULTILINK */
static int ppp_set_compress(struct ppp *ppp, unsigned long arg);
static void ppp_ccp_peek(struct ppp *ppp, struct sk_buff *skb, int inbound);
static void ppp_ccp_closed(struct ppp *ppp);
static struct compressor *find_compressor(int type);
void ppp_get_stats(struct ppp *ppp, struct ppp_stats *st);
static struct ppp *ppp_create_interface(struct net *net, int unit, int *retp);
static void init_ppp_file(struct ppp_file *pf, int kind);
static void ppp_shutdown_interface(struct ppp *ppp);
static void ppp_destroy_interface(struct ppp *ppp);
static struct ppp *ppp_find_unit(struct ppp_net *pn, int unit);
static struct channel *ppp_find_channel(struct ppp_net *pn, int unit);
static int ppp_connect_channel(struct channel *pch, int unit);
static int ppp_disconnect_channel(struct channel *pch);
static void ppp_destroy_channel(struct channel *pch);
static int unit_get(struct idr *p, void *ptr, int n);
static int unit_set(struct idr *p, void *ptr, int n);
static void unit_put(struct idr *p, int n);
static void *unit_find(struct idr *p, int n);

static struct class *ppp_class;

/* per net-namespace data */
static inline struct ppp_net *ppp_pernet(struct net *net)
{
	BUG_ON(!net);

	return net_generic(net, ppp_net_id);
}

/* Translates a PPP protocol number to a NP index (NP == network protocol) */
static inline int proto_to_npindex(int proto)
{
	switch (proto) {
	case PPP_IP:
		return NP_IP;
	case PPP_IPV6:
		return NP_IPV6;
	case PPP_IPX:
		return NP_IPX;
	case PPP_AT:
		return NP_AT;
	case PPP_MPLS_UC:
		return NP_MPLS_UC;
	case PPP_MPLS_MC:
		return NP_MPLS_MC;
	case PPP_BRIDGE:
		return NP_BRIDGE;
	}
	return -EINVAL;
}

/* Translates an NP index into a PPP protocol number */
static const int npindex_to_proto[NUM_NP] = {
	PPP_IP,
	PPP_IPV6,
	PPP_IPX,
	PPP_AT,
	PPP_MPLS_UC,
	PPP_MPLS_MC,
	PPP_BRIDGE,
};

/* Translates an ethertype into an NP index */
static inline int ethertype_to_npindex(int ethertype)
{
	switch (ethertype) {
	case ETH_P_IP:
		return NP_IP;
	case ETH_P_IPV6:
		return NP_IPV6;
	case ETH_P_IPX:
		return NP_IPX;
	case ETH_P_PPPTALK:
	case ETH_P_ATALK:
		return NP_AT;
	case ETH_P_MPLS_UC:
		return NP_MPLS_UC;
	case ETH_P_MPLS_MC:
		return NP_MPLS_MC;
	}
	return -1;
}

/* Translates an NP index into an ethertype */
static const int npindex_to_ethertype[NUM_NP] = {
	ETH_P_IP,
	ETH_P_IPV6,
	ETH_P_IPX,
	ETH_P_PPPTALK,
	ETH_P_MPLS_UC,
	ETH_P_MPLS_MC,
	ETH_P_802_3
};

/*
 * Locking shorthand.
 */
#define ppp_xmit_lock(ppp)	spin_lock_bh(&(ppp)->wlock)
#define ppp_xmit_unlock(ppp)	spin_unlock_bh(&(ppp)->wlock)
#define ppp_recv_lock(ppp)	spin_lock_bh(&(ppp)->rlock)
#define ppp_recv_unlock(ppp)	spin_unlock_bh(&(ppp)->rlock)
#define ppp_lock(ppp)		do { ppp_xmit_lock(ppp); \
				     ppp_recv_lock(ppp); } while (0)
#define ppp_unlock(ppp)		do { ppp_recv_unlock(ppp); \
				     ppp_xmit_unlock(ppp); } while (0)

static int bcp_encap(struct sk_buff **skb)
{
	unsigned char *pp;

	if (skb_headroom(*skb) < PPP_HDRLEN + BCP_HDRLEN) {
		struct sk_buff *ns;

		ns = alloc_skb((*skb)->len + PPP_HDRLEN + BCP_HDRLEN,
			       GFP_ATOMIC);
		if (ns == 0)
			return -EINVAL;
		skb_reserve(ns, PPP_HDRLEN + BCP_HDRLEN);
		skb_copy_bits(*skb, 0, skb_put(ns, (*skb)->len), (*skb)->len);
		kfree_skb(*skb);
		*skb = ns;
	}

	pp = skb_push(*skb, BCP_HDRLEN);
	pp[0] = 0; /* flags */
	pp[1] = 1; /* mactype */
	return NP_BRIDGE;
}

static int
bcp_decap(struct sk_buff *skb, struct net_device *dev)
{
	if (!pskb_may_pull(skb, BCP_HDRLEN + ETH_HLEN))
		return -1;

	if (skb->data[1] != 1)
		return -1;

	if (skb->data[0] & BCP_LAN_FCS) {
		if (skb->len < 4)
			return -1;
		pskb_trim_rcsum(skb, skb->len - 4);
	}
	skb_pull_rcsum(skb, BCP_HDRLEN);

	skb->protocol = eth_type_trans(skb, dev);

	if (is_multicast_ether_addr(eth_hdr(skb)->h_dest)) {
		if (is_broadcast_ether_addr(eth_hdr(skb)->h_dest))
			skb->pkt_type = PACKET_BROADCAST;
		else 
			skb->pkt_type = PACKET_MULTICAST;
	} else {
		skb->pkt_type = PACKET_OTHERHOST;
	}

	return 0;
}

/*
 * /dev/ppp device routines.
 * The /dev/ppp device is used by pppd to control the ppp unit.
 * It supports the read, write, ioctl and poll functions.
 * Open instances of /dev/ppp can be in one of three states:
 * unattached, attached to a ppp unit, or attached to a ppp channel.
 */
static int ppp_open(struct inode *inode, struct file *file)
{
	/*
	 * This could (should?) be enforced by the permissions on /dev/ppp.
	 */
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;
	return 0;
}

static int ppp_release(struct inode *unused, struct file *file)
{
	struct ppp_file *pf = file->private_data;
	struct ppp *ppp;

	if (pf) {
		file->private_data = NULL;
		if (pf->kind == INTERFACE) {
			ppp = PF_TO_PPP(pf);
			if (file == ppp->owner)
				ppp_shutdown_interface(ppp);
		}
		if (atomic_dec_and_test(&pf->refcnt)) {
			switch (pf->kind) {
			case INTERFACE:
				ppp_destroy_interface(PF_TO_PPP(pf));
				break;
			case CHANNEL:
				ppp_destroy_channel(PF_TO_CHANNEL(pf));
				break;
			}
		}
	}
	return 0;
}

static ssize_t ppp_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	struct ppp_file *pf = file->private_data;
	DECLARE_WAITQUEUE(wait, current);
	ssize_t ret;
	struct sk_buff *skb = NULL;
	struct iovec iov;

	ret = count;

	if (!pf)
		return -ENXIO;
	add_wait_queue(&pf->rwait, &wait);
	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		skb = skb_dequeue(&pf->rq);
		if (skb)
			break;
		ret = 0;
		if (pf->dead)
			break;
		if (pf->kind == INTERFACE) {
			/*
			 * Return 0 (EOF) on an interface that has no
			 * channels connected, unless it is looping
			 * network traffic (demand mode).
			 */
			struct ppp *ppp = PF_TO_PPP(pf);
			if (ppp->n_channels == 0 &&
			    (ppp->flags & SC_LOOP_TRAFFIC) == 0)
				break;
		}
		ret = -EAGAIN;
		if (file->f_flags & O_NONBLOCK)
			break;
		ret = -ERESTARTSYS;
		if (signal_pending(current))
			break;
		schedule();
	}
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&pf->rwait, &wait);

	if (!skb)
		goto out;

	ret = -EOVERFLOW;
	if (skb->len > count)
		goto outf;
	ret = -EFAULT;
	iov.iov_base = buf;
	iov.iov_len = count;
	if (skb_copy_datagram_iovec(skb, 0, &iov, skb->len))
		goto outf;
	ret = skb->len;

 outf:
	kfree_skb(skb);
 out:
	return ret;
}

static ssize_t ppp_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	struct ppp_file *pf = file->private_data;
	struct sk_buff *skb;
	ssize_t ret;

	if (!pf)
		return -ENXIO;
	ret = -ENOMEM;
	skb = alloc_skb(count + pf->hdrlen, GFP_KERNEL);
	if (!skb)
		goto out;
	skb_reserve(skb, pf->hdrlen);
	ret = -EFAULT;
	if (copy_from_user(skb_put(skb, count), buf, count)) {
		kfree_skb(skb);
		goto out;
	}

	switch (pf->kind) {
	case INTERFACE:
		ppp_xmit_process(PF_TO_PPP(pf), skb);
		break;
	case CHANNEL:
		skb_queue_tail(&pf->xq, skb);
		ppp_channel_push(PF_TO_CHANNEL(pf));
		break;
	}

	ret = count;

 out:
	return ret;
}

/* No kernel lock - fine */
static unsigned int ppp_poll(struct file *file, poll_table *wait)
{
	struct ppp_file *pf = file->private_data;
	unsigned int mask;

	if (!pf)
		return 0;
	poll_wait(file, &pf->rwait, wait);
	mask = POLLOUT | POLLWRNORM;
	if (skb_peek(&pf->rq))
		mask |= POLLIN | POLLRDNORM;
	if (pf->dead)
		mask |= POLLHUP;
	else if (pf->kind == INTERFACE) {
		/* see comment in ppp_read */
		struct ppp *ppp = PF_TO_PPP(pf);
		if (ppp->n_channels == 0 &&
		    (ppp->flags & SC_LOOP_TRAFFIC) == 0)
			mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

#ifdef CONFIG_PPP_FILTER
static int get_filter(void __user *arg, struct sock_filter **p)
{
	struct sock_fprog uprog;
	struct sock_filter *code = NULL;
	int len, err;

	if (copy_from_user(&uprog, arg, sizeof(uprog)))
		return -EFAULT;

	if (!uprog.len) {
		*p = NULL;
		return 0;
	}

	len = uprog.len * sizeof(struct sock_filter);
	code = memdup_user(uprog.filter, len);
	if (IS_ERR(code))
		return PTR_ERR(code);

	err = sk_chk_filter(code, uprog.len);
	if (err) {
		kfree(code);
		return err;
	}

	*p = code;
	return uprog.len;
}
#endif /* CONFIG_PPP_FILTER */

static long ppp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct ppp_file *pf = file->private_data;
	struct ppp *ppp;
	int err = -EFAULT, val, i;
	struct ppp_idle idle;
	struct npioctl npi;
	int unit, cflags;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;

	if (!pf)
		return ppp_unattached_ioctl(current->nsproxy->net_ns,
					pf, file, cmd, arg);

	if (cmd == PPPIOCDETACH) {
		/*
		 * We have to be careful here... if the file descriptor
		 * has been dup'd, we could have another process in the
		 * middle of a poll using the same file *, so we had
		 * better not free the interface data structures -
		 * instead we fail the ioctl.  Even in this case, we
		 * shut down the interface if we are the owner of it.
		 * Actually, we should get rid of PPPIOCDETACH, userland
		 * (i.e. pppd) could achieve the same effect by closing
		 * this fd and reopening /dev/ppp.
		 */
		err = -EINVAL;
		mutex_lock(&ppp_mutex);
		if (pf->kind == INTERFACE) {
			ppp = PF_TO_PPP(pf);
			if (file == ppp->owner)
				ppp_shutdown_interface(ppp);
		}
		if (atomic_long_read(&file->f_count) <= 2) {
			ppp_release(NULL, file);
			err = 0;
		} else
			pr_warn("PPPIOCDETACH file->f_count=%ld\n",
				atomic_long_read(&file->f_count));
		mutex_unlock(&ppp_mutex);
		return err;
	}

	if (pf->kind == CHANNEL) {
		struct channel *pch;
		struct ppp_channel *chan;

		mutex_lock(&ppp_mutex);
		pch = PF_TO_CHANNEL(pf);

		switch (cmd) {
		case PPPIOCCONNECT:
			if (get_user(unit, p))
				break;
			err = ppp_connect_channel(pch, unit);
			break;

		case PPPIOCDISCONN:
			err = ppp_disconnect_channel(pch);
			break;

		case PPPIOCSMTU:
			if (get_user(val, p))
				break;
			down_read(&pch->chan_sem);
			chan = pch->chan;
			err = -ENOTTY;
			if (chan) {
				err = 0;
				if (!pch->chan->mtu || val < pch->chan->mtu) {
					pch->chan->mtu = val;

					ppp = pch->ppp;
					if (ppp) {
						ppp_lock(ppp);
						if (ppp->chan_min_mtu > val)
							ppp->chan_min_mtu = val;
						ppp_unlock(ppp);
					}
				}
			}
			up_read(&pch->chan_sem);
			break;
		default:
			down_read(&pch->chan_sem);
			chan = pch->chan;
			err = -ENOTTY;
			if (chan && chan->ops->ioctl)
				err = chan->ops->ioctl(chan, cmd, arg);
			up_read(&pch->chan_sem);
		}
		mutex_unlock(&ppp_mutex);
		return err;
	}

	if (pf->kind != INTERFACE) {
		/* can't happen */
		pr_err("PPP: not interface or channel??\n");
		return -EINVAL;
	}

	mutex_lock(&ppp_mutex);
	ppp = PF_TO_PPP(pf);
	switch (cmd) {
	case PPPIOCSMRU:
		if (get_user(val, p))
			break;
		ppp->mru = val;
		err = 0;
		break;

	case PPPIOCSFLAGS:
		if (get_user(val, p))
			break;
		ppp_lock(ppp);
		cflags = ppp->flags & ~val;
		ppp->flags = val & SC_FLAG_BITS;
		ppp_unlock(ppp);
		if (cflags & SC_CCP_OPEN)
			ppp_ccp_closed(ppp);
		err = 0;
		break;

	case PPPIOCGFLAGS:
		val = ppp->flags | ppp->xstate | ppp->rstate;
		if (put_user(val, p))
			break;
		err = 0;
		break;

	case PPPIOCSCOMPRESS:
		err = ppp_set_compress(ppp, arg);
		break;

	case PPPIOCGUNIT:
		if (put_user(ppp->file.index, p))
			break;
		err = 0;
		break;

	case PPPIOCSDEBUG:
		if (get_user(val, p))
			break;
		ppp->debug = val;
		err = 0;
		break;

	case PPPIOCGDEBUG:
		if (put_user(ppp->debug, p))
			break;
		err = 0;
		break;

	case PPPIOCGIDLE:
		idle.xmit_idle = (jiffies - ppp->last_xmit) / HZ;
		idle.recv_idle = (jiffies - ppp->last_recv) / HZ;
		if (copy_to_user(argp, &idle, sizeof(idle)))
			break;
		err = 0;
		break;

	case PPPIOCGNPMODE:
	case PPPIOCSNPMODE:
		if (copy_from_user(&npi, argp, sizeof(npi)))
			break;
		err = proto_to_npindex(npi.protocol);
		if (err < 0)
			break;
		i = err;
		if (cmd == PPPIOCGNPMODE) {
			err = -EFAULT;
			npi.mode = ppp->npmode[i];
			if (copy_to_user(argp, &npi, sizeof(npi)))
				break;
		} else {
			ppp->npmode[i] = npi.mode;
			if (ppp->dev->reg_state == NETREG_UNINITIALIZED)
				register_netdev(ppp->dev);
			else 
				/* we may be able to transmit more packets now */
			netif_wake_queue(ppp->dev);
		}
		err = 0;
		break;

#ifdef CONFIG_PPP_FILTER
	case PPPIOCSPASS:
	{
		struct sock_filter *code;
		err = get_filter(argp, &code);
		if (err >= 0) {
			ppp_lock(ppp);
			kfree(ppp->pass_filter);
			ppp->pass_filter = code;
			ppp->pass_len = err;
			ppp_unlock(ppp);
			err = 0;
		}
		break;
	}
	case PPPIOCSACTIVE:
	{
		struct sock_filter *code;
		err = get_filter(argp, &code);
		if (err >= 0) {
			ppp_lock(ppp);
			kfree(ppp->active_filter);
			ppp->active_filter = code;
			ppp->active_len = err;
			ppp_unlock(ppp);
			err = 0;
		}
		break;
	}
#endif /* CONFIG_PPP_FILTER */

#ifdef CONFIG_PPP_MULTILINK
	case PPPIOCSMRRU:
		if (get_user(val, p))
			break;
		ppp_recv_lock(ppp);
		ppp->mrru = val;
		ppp_recv_unlock(ppp);
		err = 0;
		break;
#endif /* CONFIG_PPP_MULTILINK */

	case PPPIOCSUSER:
		if (copy_from_user(&ppp->userid, (unsigned *) arg,
				   sizeof(ppp->userid))) break;
		err = 0;
		break;

	default:
		err = -ENOTTY;
	}
	mutex_unlock(&ppp_mutex);
	return err;
}

static int ppp_unattached_ioctl(struct net *net, struct ppp_file *pf,
			struct file *file, unsigned int cmd, unsigned long arg)
{
	int unit, err = -EFAULT;
	struct ppp *ppp;
	struct channel *chan;
	struct ppp_net *pn;
	int __user *p = (int __user *)arg;

	mutex_lock(&ppp_mutex);
	switch (cmd) {
	case PPPIOCNEWUNIT:
		/* Create a new ppp unit */
		if (get_user(unit, p))
			break;
		ppp = ppp_create_interface(net, unit, &err);
		if (!ppp)
			break;
		file->private_data = &ppp->file;
		ppp->owner = file;
		err = -EFAULT;
		if (put_user(ppp->file.index, p))
			break;
		err = 0;
		break;

	case PPPIOCATTACH:
		/* Attach to an existing ppp unit */
		if (get_user(unit, p))
			break;
		err = -ENXIO;
		pn = ppp_pernet(net);
		mutex_lock(&pn->all_ppp_mutex);
		ppp = ppp_find_unit(pn, unit);
		if (ppp) {
			atomic_inc(&ppp->file.refcnt);
			file->private_data = &ppp->file;
			err = 0;
		}
		mutex_unlock(&pn->all_ppp_mutex);
		break;

	case PPPIOCATTCHAN:
		if (get_user(unit, p))
			break;
		err = -ENXIO;
		pn = ppp_pernet(net);
		spin_lock_bh(&pn->all_channels_lock);
		chan = ppp_find_channel(pn, unit);
		if (chan) {
			atomic_inc(&chan->file.refcnt);
			file->private_data = &chan->file;
			err = 0;
		}
		spin_unlock_bh(&pn->all_channels_lock);
		break;

	default:
		err = -ENOTTY;
	}
	mutex_unlock(&ppp_mutex);
	return err;
}

#ifdef CONFIG_COMPAT
struct ppp_option_data32 {
	compat_caddr_t	ptr;
	u32			length;
	compat_int_t		transmit;
};
#define PPPIOCSCOMPRESS32	_IOW('t', 77, struct ppp_option_data32)

static long ppp_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == PPPIOCSCOMPRESS32) {
	struct ppp_option_data32 __user *odata32 = compat_ptr(arg);
	struct ppp_option_data __user *odata;
	__u32 data;
	void __user *datap;

	odata = compat_alloc_user_space(sizeof(*odata));

	if (get_user(data, &odata32->ptr))
		return -EFAULT;

	datap = compat_ptr(data);
	if (put_user(datap, &odata->ptr))
		return -EFAULT;

	if (copy_in_user(&odata->length, &odata32->length,
			 sizeof(__u32) + sizeof(int)))
		return -EFAULT;

	return ppp_ioctl(file, PPPIOCSCOMPRESS, (unsigned long) odata);
    }
    return ppp_ioctl(file, cmd, arg);
}
#endif

static const struct file_operations ppp_device_fops = {
	.owner		= THIS_MODULE,
	.read		= ppp_read,
	.write		= ppp_write,
	.poll		= ppp_poll,
	.unlocked_ioctl	= ppp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = ppp_compat_ioctl,
#endif
	.open		= ppp_open,
	.release	= ppp_release,
	.llseek		= noop_llseek,
};

static void unreg_devices(struct work_struct *work)
{
	struct ppp_net *pn = container_of(work, struct ppp_net, unreg_work.work);
	struct net_device *dev;
	struct net_device *next;
	struct list_head list;

	rtnl_lock();
	list_replace_init(&pn->unreg_list, &list);
	unregister_netdevice_many(&list);
	rtnl_unlock();

	list_for_each_entry_safe(dev, next, &list, unreg_list) {
		struct ppp *ppp = netdev_priv(dev);

		if (atomic_dec_and_test(&ppp->file.refcnt))
			ppp_destroy_interface(ppp);
	}
}

static __net_init int ppp_init_net(struct net *net)
{
	struct ppp_net *pn = net_generic(net, ppp_net_id);

	idr_init(&pn->units_idr);
	mutex_init(&pn->all_ppp_mutex);

	INIT_LIST_HEAD(&pn->all_channels);
	INIT_LIST_HEAD(&pn->new_channels);

	spin_lock_init(&pn->all_channels_lock);

	INIT_LIST_HEAD(&pn->unreg_list);
	INIT_DELAYED_WORK(&pn->unreg_work, unreg_devices);

	return 0;
}

static __net_exit void ppp_exit_net(struct net *net)
{
	struct ppp_net *pn = net_generic(net, ppp_net_id);

	idr_destroy(&pn->units_idr);
}

static struct pernet_operations ppp_net_ops = {
	.init = ppp_init_net,
	.exit = ppp_exit_net,
	.id   = &ppp_net_id,
	.size = sizeof(struct ppp_net),
};

#define PPP_MAJOR	108

/* Called at boot time if ppp is compiled into the kernel,
   or at module load time (from init_module) if compiled as a module. */
static int __init ppp_init(void)
{
	int err;

	pr_info("PPP generic driver version " PPP_VERSION "\n");

	err = register_pernet_device(&ppp_net_ops);
	if (err) {
		pr_err("failed to register PPP pernet device (%d)\n", err);
		goto out;
	}

	err = register_chrdev(PPP_MAJOR, "ppp", &ppp_device_fops);
	if (err) {
		pr_err("failed to register PPP device (%d)\n", err);
		goto out_net;
	}

	ppp_class = class_create(THIS_MODULE, "ppp");
	if (IS_ERR(ppp_class)) {
		err = PTR_ERR(ppp_class);
		goto out_chrdev;
	}

	/* not a big deal if we fail here :-) */
	device_create(ppp_class, NULL, MKDEV(PPP_MAJOR, 0), NULL, "ppp");

	return 0;

out_chrdev:
	unregister_chrdev(PPP_MAJOR, "ppp");
out_net:
	unregister_pernet_device(&ppp_net_ops);
out:
	return err;
}

static inline unsigned optlen(const unsigned char *opt, unsigned offset)
{
	return opt[offset] >= 2 && opt[offset + 1] ? opt[offset + 1] : 1;
}

static void change_mss(struct ppp *ppp, struct sk_buff *skb, unsigned offset)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned tcphoff;
	unsigned char *opt;
	unsigned hlen;
	unsigned i;
	unsigned newmss;
	unsigned lenword;
	unsigned tot_len;

	if (!(ppp->flags & SC_CHANGE_MSS))
		return;
	
	newmss = min(ppp->chan_min_mtu, ppp->mru) - 40;
	/* reduce MSS by extra 6 bytes, in case MLPPP is enabled */
	if (ppp->flags & SC_MULTILINK)
		newmss -= 6;

	if (skb->protocol != htons(ETH_P_IP))
		return;

	iph = (struct iphdr *) (skb->data + offset);

	if (!pskb_may_pull(skb, (unsigned char *) &iph[1] - skb->data))
		return;

	if (iph->frag_off & htons(IP_OFFSET))
		return;
	
	if (iph->ihl < 5 || iph->version != 4)
		return;
	
	if (iph->protocol != IPPROTO_TCP)
		return;

	tcphoff = iph->ihl * 4 + offset;
	tcph = (struct tcphdr *) (skb->data + tcphoff);
	
	if (!pskb_may_pull(skb, (unsigned char *) &tcph[1] - skb->data))
		return;

	if (!tcph->syn)
		return;

	if (!skb_make_writable(skb, skb->len))
		return;

	opt = (unsigned char *) tcph;
	hlen = tcph->doff * 4;

	if (!pskb_may_pull(skb, opt + hlen - skb->data))
		return;

	for (i = sizeof(*tcph); i < hlen; i += optlen(opt, i)) {
		if (opt[i] == TCPOPT_MSS && (hlen - i) >= TCPOLEN_MSS &&
		    opt[i + 1] == TCPOLEN_MSS) {
			unsigned mss = (opt[i + 2] << 8) | opt[i + 3];
			if (mss <= newmss)
				return;

			opt[i + 2] = (newmss & 0xff00) >> 8;
			opt[i + 3] = newmss & 0x00ff;

			inet_proto_csum_replace2(&tcph->check, skb,
						 htons(mss), htons(newmss),
						 0);
			return;
		}
	}
	
	/* MSS option not found. Need to add it */
	
	/* ignore TCP SYN with extra data */
	if (opt - skb->data + hlen < skb->len)
		return;

	// FIXME: kill me?
	if (skb_tailroom(skb) < TCPOLEN_MSS) {
		if (pskb_expand_head(skb, 0,
				     TCPOLEN_MSS - skb_tailroom(skb),
				     GFP_ATOMIC))
			return;
		iph = (struct iphdr *) (skb->data + offset);
		tcph = (struct tcphdr *) (skb->data + tcphoff);
	}

	skb_put(skb, TCPOLEN_MSS);

	opt = (unsigned char *) tcph + sizeof(*tcph);
	memmove(opt + TCPOLEN_MSS, opt, hlen - sizeof(struct tcphdr));

	inet_proto_csum_replace2(&tcph->check, skb,
				 htons(hlen), htons(hlen + TCPOLEN_MSS), 1);
	opt[0] = TCPOPT_MSS;
	opt[1] = TCPOLEN_MSS;
	opt[2] = (newmss & 0xff00) >> 8;
	opt[3] = newmss & 0x00ff;

	inet_proto_csum_replace4(&tcph->check, skb, 0,
				 get_unaligned((unsigned *) opt), 0);

	lenword = ((unsigned short *) tcph)[6];
	tcph->doff += TCPOLEN_MSS / 4;
	inet_proto_csum_replace2(&tcph->check, skb,
				 lenword, ((unsigned short *) tcph)[6], 0);

	tot_len = htons(ntohs(iph->tot_len) + TCPOLEN_MSS);
	csum_replace2(&iph->check, iph->tot_len, tot_len);
	iph->tot_len = tot_len;
}

/*
 * Network interface unit routines.
 */
netdev_tx_t
ppp_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ppp *ppp = netdev_priv(dev);
	int npi, proto;
	unsigned char *pp;

	change_mss(ppp, skb, skb_network_offset(skb));

	if (skb_network_header(skb) > skb->data)
		npi = bcp_encap(&skb);
	else {
	npi = ethertype_to_npindex(ntohs(skb->protocol));
		if (npi >= 0 && ppp->npmode[npi] != NPMODE_PASS &&
		    ppp->npmode[NP_BRIDGE] == NPMODE_PASS) {
		    npi = bcp_encap(&skb);
		}
		if (npi < 0) npi = bcp_encap(&skb);
	}

	if (npi < 0)
		goto outf;

	/* Drop, accept or reject the packet */
	switch (ppp->npmode[npi]) {
	case NPMODE_PASS:
		break;
	case NPMODE_QUEUE:
		/* it would be nice to have a way to tell the network
		   system to queue this one up for later. */
		goto outf;
	case NPMODE_DROP:
	case NPMODE_ERROR:
		goto outf;
	}

	/* Put the 2-byte PPP protocol number on the front,
	   making sure there is room for the address and control fields. */
	if (skb_cow_head(skb, PPP_HDRLEN))
		goto outf;

	pp = skb_push(skb, 2);
	proto = npindex_to_proto[npi];
	put_unaligned_be16(proto, pp);

	ppp_xmit_process(ppp, skb);
	return NETDEV_TX_OK;

 outf:
	kfree_skb(skb);
	if (ppp->dev)
		atomic_inc((atomic_t *) &ppp->dev->stats.tx_dropped);
	return NETDEV_TX_OK;
}
EXPORT_SYMBOL(ppp_start_xmit);

int ppp_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct ppp *ppp = netdev_priv(dev);
	int err = -EFAULT;
	void __user *addr = (void __user *) ifr->ifr_ifru.ifru_data;
	struct ppp_stats stats;
	struct ppp_comp_stats cstats;
	char *vers;

	switch (cmd) {
	case SIOCGPPPSTATS:
		ppp_get_stats(ppp, &stats);
		if (copy_to_user(addr, &stats, sizeof(stats)))
			break;
		err = 0;
		break;

	case SIOCGPPPCSTATS:
		memset(&cstats, 0, sizeof(cstats));
		if (ppp->xc_state)
			ppp->xcomp->comp_stat(ppp->xc_state, &cstats.c);
		if (ppp->rc_state)
			ppp->rcomp->decomp_stat(ppp->rc_state, &cstats.d);
		if (copy_to_user(addr, &cstats, sizeof(cstats)))
			break;
		err = 0;
		break;

	case SIOCGPPPVER:
		vers = PPP_VERSION;
		if (copy_to_user(addr, vers, strlen(vers) + 1))
			break;
		err = 0;
		break;

	default:
		err = -EINVAL;
	}

	return err;
}
EXPORT_SYMBOL(ppp_net_ioctl);

static const struct net_device_ops ppp_netdev_ops = {
	.ndo_start_xmit = ppp_start_xmit,
	.ndo_do_ioctl   = ppp_net_ioctl,
};

static void ppp_setup(struct net_device *dev)
{
	dev->netdev_ops = &ppp_netdev_ops;
	dev->hard_header_len = PPP_HDRLEN;
	dev->mtu = PPP_MTU;
	dev->addr_len = 0;
	dev->tx_queue_len = 0;
	dev->type = ARPHRD_PPP;
	dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
	dev->features |= NETIF_F_NETNS_LOCAL | NETIF_F_LLTX;
	dev->priv_flags &= ~IFF_XMIT_DST_RELEASE;
}

/*
 * Transmit-side routines.
 */

/*
 * Called to do any work queued up on the transmit side
 * that can now be done.
 */
static void
ppp_xmit_process(struct ppp *ppp, struct sk_buff *skb)
{
	int lock = 0;
	int comp = 0;

	if (ppp->closing) 
		return;

	if (skb) {
		int proto = PPP_PROTO(skb);

		/*
		 * If we are waiting for traffic (demand dialling),
		 * queue it up for pppd to receive.
		 */
		if (ppp->flags & SC_LOOP_TRAFFIC) {
			if (ppp->file.rq.qlen > PPP_MAX_RQLEN) {
				kfree_skb(skb);
				return;
			}
			skb_queue_tail(&ppp->file.rq, skb);
			wake_up_interruptible(&ppp->file.rwait);
			return;
		}

		if (proto == PPP_CCP) {
	ppp_xmit_lock(ppp);
			ppp_ccp_peek(ppp, skb, 0);
			ppp_xmit_unlock(ppp);
		} else if (proto != PPP_LCP && (ppp->xstate & SC_COMP_RUN)) {
			struct compressor *xcomp = rcu_dereference(ppp->xcomp);

			if (!xcomp ||
			    (!(ppp->flags & SC_CCP_UP) && (ppp->flags & SC_MUST_COMP)))
				return;

			if (!xcomp->lockless) lock = 1;
			comp = 1;
		}
	}

	if (lock) ppp_xmit_lock(ppp);

	if (skb) {
		if (comp) {
			skb = pad_compress_skb(ppp, skb);
			if (!skb) {
				if (lock) ppp_xmit_unlock(ppp);
				return;
			}
		}

		atomic_inc((atomic_t *) &ppp->dev->stats.tx_packets);
		atomic_add(skb->len - 2, (atomic_t *) &ppp->dev->stats.tx_bytes);
		ppp->last_xmit = jiffies;

		if (!lock && skb_queue_empty(&ppp->file.xq)) {
			if (ppp->flags & SC_MULTILINK ? !ppp_mp_explode(ppp, skb)
			    : !ppp_push(ppp, skb)) {
				skb_queue_tail(&ppp->file.xq, skb);
			}
			return;
		}
		skb_queue_tail(&ppp->file.xq, skb);
	}

	while ((skb = skb_dequeue(&ppp->file.xq))) {
		if (ppp->flags & SC_MULTILINK ? !ppp_mp_explode(ppp, skb)
		    : !ppp_push(ppp, skb)) {
			skb_queue_head(&ppp->file.xq, skb);
			break;
		}
	}

	if (lock) ppp_xmit_unlock(ppp);

		/* If there's no work left to do, tell the core net
		   code that we can accept some more. */
	if (ppp->dev->reg_state != NETREG_UNINITIALIZED) {
		if (skb_queue_empty(&ppp->file.xq))
			netif_wake_queue(ppp->dev);
		else
			netif_stop_queue(ppp->dev);
	}
}

static inline struct sk_buff *
pad_compress_skb(struct ppp *ppp, struct sk_buff *skb)
{
	struct sk_buff *new_skb = NULL;
	struct compressor *xcomp = rcu_dereference(ppp->xcomp);
	void *xc_state = rcu_dereference(ppp->xc_state);
	int len = 0;
	int new_skb_size;
	int compressor_skb_size;

	if (!xcomp || !xc_state) {
		kfree_skb(skb);
		return NULL;
	}

	if (xcomp->inplace &&
	    xcomp->comp_extra + 2 <= skb_headroom(skb) &&
	    !skb_shared(skb) && !skb_cloned(skb)) {
		unsigned char *orgdata = skb->data - 2;
		unsigned orglen = skb->len + 2;
		__skb_push(skb, xcomp->comp_extra + 2);

		len = xcomp->compress(xc_state, orgdata,
				      skb->data, orglen, skb->len);

		if (len > 0 && (ppp->flags & SC_CCP_UP)) {
			skb_pull(skb, 2);
			return skb;
		}
		if (len == 0) {
			skb_pull(skb, xcomp->comp_extra + 2);
			return skb;
		}

		if (net_ratelimit())
			netdev_err(ppp->dev, "ppp: compressor dropped pkt\n");
		kfree_skb(skb);
		return NULL;
	}

	new_skb_size = ppp->dev->mtu +
		xcomp->comp_extra + ppp->dev->hard_header_len;
	compressor_skb_size = ppp->dev->mtu +
		xcomp->comp_extra + PPP_HDRLEN;
	new_skb = alloc_skb(new_skb_size, GFP_ATOMIC);
	if (!new_skb) {
		if (net_ratelimit())
			netdev_err(ppp->dev, "PPP: no memory (comp pkt)\n");
		kfree_skb(skb);
		return NULL;
	}
	if (ppp->dev->hard_header_len > PPP_HDRLEN)
		skb_reserve(new_skb,
			    ppp->dev->hard_header_len - PPP_HDRLEN);

	/* compressor still expects A/C bytes in hdr */
	len = xcomp->compress(xc_state, skb->data - 2,
				   new_skb->data, skb->len + 2,
				   compressor_skb_size);
	if (len > 0 && (ppp->flags & SC_CCP_UP)) {
		kfree_skb(skb);
		skb = new_skb;
		skb_put(skb, len);
		skb_pull(skb, 2);	/* pull off A/C bytes */
	} else if (len == 0) {
		/* didn't compress, or CCP not up yet */
		kfree_skb(new_skb);
		new_skb = skb;
	} else {
		/*
		 * (len < 0)
		 * MPPE requires that we do not send unencrypted
		 * frames.  The compressor will return -1 if we
		 * should drop the frame.  We cannot simply test
		 * the compress_proto because MPPE and MPPC share
		 * the same number.
		 */
		if (net_ratelimit())
			netdev_err(ppp->dev, "ppp: compressor dropped pkt\n");
		kfree_skb(skb);
		kfree_skb(new_skb);
		new_skb = NULL;
	}
	return new_skb;
}

static int
ppp_push(struct ppp *ppp, struct sk_buff *skb)
{
	struct channel *pch;
	int rc = 0;

		/* not doing multilink: send it down the first channel */
	rcu_read_lock();
	list_for_each_entry_rcu(pch, &ppp->channels, clist) {
		struct ppp_channel *chan = rcu_dereference(pch->chan);
		if (chan) {
			if (!chan->ops->lockless) spin_lock_bh(&pch->downl);
			rc = chan->ops->start_xmit(chan, skb);
			if (!chan->ops->lockless) spin_unlock_bh(&pch->downl);
		} else {
			/* channel got unregistered */
			kfree_skb(skb);
			rc = 1;
		}
		break;
	}
	rcu_read_unlock();
	return rc;
}

#ifdef CONFIG_PPP_MULTILINK
/*
 * Divide a packet to be transmitted into fragments and
 * send them out the individual links.
 */
static int ppp_mp_explode(struct ppp *ppp, struct sk_buff *skb)
{
	unsigned n_channels = ppp->n_channels;
	struct sk_buff *frag;
	unsigned char *p;
	unsigned len;
	unsigned flen;
	unsigned mtu;
	unsigned hdrlen;
	unsigned bits;
	unsigned seq;
	unsigned frags;
	unsigned rounds;
	unsigned nxchan;

	if (n_channels == 0)
		return 0;

	hdrlen = (ppp->flags & SC_MP_XSHORTSEQ)? MPHDRLEN_SSN: MPHDRLEN;

	mtu = ppp->chan_min_mtu - hdrlen;

	p = skb->data;
	len = skb->len;
	
	flen = len;
	frags = 1;
	if (len > mtu + hdrlen) {
		frags = (len + mtu - 1) / mtu;
		flen = mtu;
	}
	
	nxchan = atomic_add_return(frags, (atomic_t *) &ppp->nxchan) - frags;
	nxchan %= n_channels;

	if (frags == 1) {
		struct channel *pch;
		int rc = 0;
		/* no fragmentation, no MP protocol pass */
		rcu_read_lock();
		list_for_each_entry_rcu(pch, &ppp->channels, clist) {
			struct ppp_channel *chan;
		    
			if (nxchan > 0) {
				--nxchan;
				continue;
	}

			chan = rcu_dereference(pch->chan);
			if (!chan) continue;

			/* send down */
			if (!chan->ops->lockless) spin_lock_bh(&pch->downl);
			rc = chan->ops->start_xmit(chan, skb);
			if (!chan->ops->lockless) spin_unlock_bh(&pch->downl);
			break;
		}
		rcu_read_unlock();
		return rc;
	}

	seq = atomic_add_return(frags, (atomic_t *) &ppp->nxseq) - frags;
	bits = B;

	rounds = 0;
	while (len > 0) {
		struct channel *pch;
		int sent = 0;

		rcu_read_lock();
		list_for_each_entry_rcu(pch, &ppp->channels, clist) {
			struct ppp_channel *chan;
			unsigned char *q;
			int rc;

			if (nxchan > 0) {
				--nxchan;
			continue;
		}

			chan = rcu_dereference(pch->chan);
			if (!chan) continue;

			if (flen >= len) {
			flen = len;
				bits |= E;
		}

			frag = dev_alloc_skb(flen + hdrlen);
		if (!frag)
			goto noskb;
		q = skb_put(frag, flen + hdrlen);

		/* make the MP header */
		put_unaligned_be16(PPP_MP, q);
		if (ppp->flags & SC_MP_XSHORTSEQ) {
				q[2] = bits + ((seq >> 8) & 0xf);
				q[3] = seq;
		} else {
			q[2] = bits;
				q[3] = seq >> 16;
				q[4] = seq >> 8;
				q[5] = seq;
		}
		memcpy(q + hdrlen, p, flen);

			/* send down */
			if (!chan->ops->lockless) spin_lock_bh(&pch->downl);
			rc = chan->ops->start_xmit(chan, frag);
			if (!chan->ops->lockless) spin_unlock_bh(&pch->downl);
			if (!rc)
				goto send_err;

			sent = 1;
			++seq;
		len -= flen;
			p += flen;
		bits = 0;

			if (len == 0) {
				rcu_read_lock();
				goto done;
			}
	}
		rcu_read_unlock();

		if (!sent) {
			/* we are stuck in the loop with no channels */
			if (++rounds == 2)
				goto noskb;
		} else {
			rounds = 0;
		}
	}
  done:
	kfree_skb(skb);
	return 1;

  send_err:
	kfree_skb(frag);
 noskb:
	rcu_read_unlock();
	atomic_inc((atomic_t *) &ppp->dev->stats.tx_errors);
	kfree_skb(skb);
	return 1;
}
#endif /* CONFIG_PPP_MULTILINK */

/*
 * Try to send data out on a channel.
 */
static void
ppp_channel_push(struct channel *pch)
{
	struct sk_buff *skb;
	struct ppp *ppp;

	spin_lock_bh(&pch->downl);
	if (pch->chan) {
		while (!skb_queue_empty(&pch->file.xq)) {
			skb = skb_dequeue(&pch->file.xq);
			if (!pch->chan->ops->start_xmit(pch->chan, skb)) {
				/* put the packet back and try again later */
				skb_queue_head(&pch->file.xq, skb);
				break;
			}
		}
	} else {
		/* channel got deregistered */
		skb_queue_purge(&pch->file.xq);
	}
	spin_unlock_bh(&pch->downl);
	/* see if there is anything from the attached unit to be sent */
	if (skb_queue_empty(&pch->file.xq)) {
		read_lock_bh(&pch->upl);
		ppp = pch->ppp;
		if (ppp)
			ppp_xmit_process(ppp, NULL);
		read_unlock_bh(&pch->upl);
	}
}

/*
 * Receive-side routines.
 */

struct ppp_mp_skb_parm {
	unsigned long	timestamp;
	u32		sequence;
	u8		BEbits;
};
#define PPP_MP_CB(skb)	((struct ppp_mp_skb_parm *)((skb)->cb))

static inline void
ppp_do_recv(struct ppp *ppp, struct sk_buff *skb, struct channel *pch)
{
	if (!ppp->closing)
		ppp_receive_frame(ppp, skb, pch);
	else
		kfree_skb(skb);
}

void
ppp_input(struct ppp_channel *chan, struct sk_buff *skb)
{
	struct channel *pch;
	int proto;

	rcu_read_lock_bh();
	pch = rcu_dereference(chan->ppp);
	if (!pch) {
		kfree_skb(skb);
		rcu_read_unlock_bh();
		return;
	}

	if (!pskb_may_pull(skb, 2)) {
		rcu_read_unlock_bh();
		kfree_skb(skb);
		if (pch->ppp) {
			atomic_inc((atomic_t *) &pch->ppp->dev->stats.rx_length_errors);
			atomic_inc((atomic_t *) &pch->ppp->dev->stats.rx_errors);
		}
		return;
	}

	proto = PPP_PROTO(skb);
	if (!rcu_dereference(pch->ppp) || proto >= 0xc000 || proto == PPP_CCPFRAG) {
		/* put it on the channel queue */
		skb_queue_tail(&pch->file.rq, skb);
		/* drop old frames if queue too long */
		while (pch->file.rq.qlen > PPP_MAX_RQLEN &&
		       (skb = skb_dequeue(&pch->file.rq)))
			kfree_skb(skb);
		wake_up_interruptible(&pch->file.rwait);
	} else {
		ppp_do_recv(pch->ppp, skb, pch);
	}
	rcu_read_unlock_bh();
}

/* Put a 0-length skb in the receive queue as an error indication */
void
ppp_input_error(struct ppp_channel *chan, int code)
{
	struct channel *pch = chan->ppp;
	struct sk_buff *skb;

	if (!pch)
		return;

	read_lock_bh(&pch->upl);
	if (pch->ppp) {
		skb = alloc_skb(0, GFP_ATOMIC);
		if (skb) {
			skb->len = 0;		/* probably unnecessary */
			skb->cb[0] = code;
			ppp_do_recv(pch->ppp, skb, pch);
		}
	}
	read_unlock_bh(&pch->upl);
}

/*
 * We come in here to process a received frame.
 * The receive side of the ppp unit is locked.
 */
static void
ppp_receive_frame(struct ppp *ppp, struct sk_buff *skb, struct channel *pch)
{
	/* note: a 0-length skb is used as an error indication */
	if (skb->len > 0) {
#ifdef CONFIG_PPP_MULTILINK
		/* XXX do channel-level decompression here */
		if (PPP_PROTO(skb) == PPP_MP) {
			ppp_receive_mp_frame(ppp, skb, pch);
		} else
#endif /* CONFIG_PPP_MULTILINK */
			ppp_receive_nonmp_frame(ppp, skb);
	} else {
		kfree_skb(skb);
		atomic_inc((atomic_t *) &ppp->dev->stats.rx_errors);
	}
}

static void
ppp_receive_nonmp_frame(struct ppp *ppp, struct sk_buff *skb)
{
	struct sk_buff *ns;
	int proto, npi;

	/*
	 * Decompress the frame, if compressed.
	 * Note that some decompressors need to see uncompressed frames
	 * that come in as well as compressed frames.
	 */
	if ((ppp->rstate & SC_DECOMP_RUN) && ppp->rc_state &&
	    (ppp->rstate & (SC_DC_FERROR)) == 0) {
		ns = ppp_decompress_frame(ppp, skb);
		if (!ns) goto err;
		skb = ns;
			}
	if ((long) skb->data & 1) {
		/* align packet data, in case MRRU has messed up */
		ns = dev_alloc_skb(skb->len + 128);
		if (ns) {
			skb_copy_bits(skb, 0, skb_put(ns, skb->len), skb->len);
			kfree_skb(skb);
			skb = ns;
		}
		}

	if (ppp->flags & SC_MUST_COMP && ppp->rstate & SC_DC_FERROR)
			goto err;

	proto = PPP_PROTO(skb);
	switch (proto) {
	case PPP_CCP:
		ppp_recv_lock(ppp);
		ppp_ccp_peek(ppp, skb, 1);
		ppp_recv_unlock(ppp);
		break;
	}

	atomic_inc((atomic_t *) &ppp->dev->stats.rx_packets);
	atomic_add(skb->len - 2, (atomic_t *) &ppp->dev->stats.rx_bytes);

	npi = proto_to_npindex(proto);
	if (npi < 0) {
		/* control or unknown frame - pass it to pppd */
		skb_queue_tail(&ppp->file.rq, skb);
		/* limit queue length by dropping old frames */
		while (ppp->file.rq.qlen > PPP_MAX_RQLEN &&
		       (skb = skb_dequeue(&ppp->file.rq)))
			kfree_skb(skb);
		/* wake up any process polling or blocking on read */
		wake_up_interruptible(&ppp->file.rwait);

	} else {
		/* network protocol frame - give it to the kernel */

#ifdef CONFIG_PPP_FILTER
		/* check if the packet passes the pass and active filters */
		/* the filter instructions are constructed assuming
		   a four-byte PPP header on each packet */
		ppp_recv_lock(ppp);
		if (ppp->pass_filter || ppp->active_filter) {
			if (skb_cloned(skb) &&
			    pskb_expand_head(skb, 0, 0, GFP_ATOMIC))
				goto err;

			*skb_push(skb, 2) = 0;
			if (ppp->pass_filter &&
			    sk_run_filter(skb, ppp->pass_filter) == 0) {
				if (ppp->debug & 1)
					netdev_printk(KERN_DEBUG, ppp->dev,
						      "PPP: inbound frame "
						      "not passed\n");
				kfree_skb(skb);
				return;
			}
			if (!(ppp->active_filter &&
			      sk_run_filter(skb, ppp->active_filter) == 0))
				ppp->last_recv = jiffies;
			__skb_pull(skb, 2);
			ppp_recv_unlock(ppp);
		} else
			ppp_recv_unlock(ppp);
#endif /* CONFIG_PPP_FILTER */
			ppp->last_recv = jiffies;

		if ((ppp->dev->flags & IFF_UP) == 0 ||
		    ppp->npmode[npi] != NPMODE_PASS) {
			kfree_skb(skb);
		} else {
			/* chop off protocol */
			skb_pull_rcsum(skb, 2);
			skb->dev = ppp->dev;
			if (npi == NP_BRIDGE) {
				if (bcp_decap(skb, ppp->dev) != 0)
					goto err;
			} else {
			skb->protocol = htons(npindex_to_ethertype[npi]);
			skb_reset_mac_header(skb);
			}
			change_mss(ppp, skb, 0);
			netif_rx(skb);
		}
	}
	return;

 err:
	kfree_skb(skb);
	atomic_inc((atomic_t *) &ppp->dev->stats.rx_errors);
}

static struct sk_buff *
ppp_decompress_frame(struct ppp *ppp, struct sk_buff *skb)
{
	int proto = PPP_PROTO(skb);
	struct sk_buff *ns = NULL;
	struct compressor *rcomp = rcu_dereference(ppp->rcomp);
	void *rc_state = rcu_dereference(ppp->rc_state);
	int len;

	if (!rcomp || !rc_state)
		goto err;

	/* Until we fix all the decompressor's need to make sure
	 * data portion is linear.
	 */
	if (!pskb_may_pull(skb, skb->len))
		goto err;

	if (proto == PPP_COMP) {
		int obuff_size;

		if (rcomp->inplace && ((long) skb->data & 1) == 0) {
			if (!rcomp->lockless) ppp_recv_lock(ppp);
			len = rcomp->decompress(rc_state,
						skb->data - 2, skb->len + 2,
						skb->data - 2, skb->len + 2);
			if (!rcomp->lockless) ppp_recv_unlock(ppp);
			if (len <= 0)
				goto decomp_err;
			skb_put(skb, len - 2 - skb->len);
			return skb;
		}

		switch(rcomp->compress_proto) {
		case CI_MPPE:
			obuff_size = ppp->mru + PPP_HDRLEN + 1;
			break;
		default:
			obuff_size = ppp->mru + PPP_HDRLEN;
			break;
		}

		ns = dev_alloc_skb(obuff_size);
		if (!ns) {
			netdev_err(ppp->dev, "ppp_decompress_frame: "
				   "no memory\n");
			goto err;
		}
		/* the decompressor still expects the A/C bytes in the hdr */
		if (!rcomp->lockless) ppp_recv_lock(ppp);
		len = rcomp->decompress(rc_state, skb->data - 2,
				skb->len + 2, ns->data, obuff_size);
		if (!rcomp->lockless) ppp_recv_unlock(ppp);
		if (len <= 0) {
			/* Pass the compressed frame to pppd as an
			   error indication. */
		  decomp_err:
			if (len == DECOMP_FATALERROR)
				ppp->rstate |= SC_DC_FERROR;
			kfree_skb(ns);
			if (len == 0 || (ppp->rstate & SC_DC_ERROR))
				return NULL;
			goto err;
		}

		kfree_skb(skb);
		skb = ns;
		skb_put(skb, len);
		skb_pull(skb, 2);	/* pull off the A/C bytes */

	} else {
		/* Uncompressed frame - pass to decompressor so it
		   can update its dictionary if necessary. */
		ppp_recv_lock(ppp);
		if (rcomp->incomp)
			rcomp->incomp(rc_state, skb->data - 2,
					   skb->len + 2);
		ppp_recv_unlock(ppp);
	}

	return skb;

 err:
	ppp->rstate |= SC_DC_ERROR;
	return skb;
}

#ifdef CONFIG_PPP_MULTILINK
/*
 * Receive a multilink frame.
 * We put it on the reconstruction queue and then pull off
 * as many completed frames as we can.
 */
static void
ppp_receive_mp_frame(struct ppp *ppp, struct sk_buff *skb, struct channel *pch)
{
	struct sk_buff *p;
	struct sk_buff_head *list = &ppp->mrq;
	int mphdrlen = (ppp->flags & SC_MP_SHORTSEQ)? MPHDRLEN_SSN: MPHDRLEN;
	u32 seq;

	if (!pskb_may_pull(skb, mphdrlen + 1) || ppp->mrru == 0)
		goto err;		/* no good, throw it away */

	/* Decode sequence number and begin/end bits */
	if (ppp->flags & SC_MP_SHORTSEQ) {
		seq = ((skb->data[2] & 0x0f) << 8) | skb->data[3];
	} else {
		seq = (skb->data[3] << 16) | (skb->data[4] << 8)| skb->data[5];
	}
	PPP_MP_CB(skb)->BEbits = skb->data[2];
	skb_pull(skb, mphdrlen);	/* pull off PPP and MP headers */

	/*
	 * Do protocol ID decompression on the first fragment of each packet.
	 */
	if ((PPP_MP_CB(skb)->BEbits & B) && (skb->data[0] & 1))
		*skb_push(skb, 1) = 0;

	PPP_MP_CB(skb)->sequence = seq;
	PPP_MP_CB(skb)->timestamp = jiffies;

	ppp_recv_lock(ppp);
	/* Put the fragment on the reconstruction queue */
	skb_queue_walk(list, p) {
		if (seq_before(seq, PPP_MP_CB(p)->sequence))
			break;
	}
	__skb_queue_before(list, p, skb);

	/* Pull completed packets off the queue and receive them. */
	while ((skb = ppp_mp_reconstruct(ppp))) {
		if (skb->len <= ppp->mrru + 2 && pskb_may_pull(skb, 2)) {
			ppp_recv_unlock(ppp);
			ppp_receive_nonmp_frame(ppp, skb);
			ppp_recv_lock(ppp);
		}
		else {
			++ppp->dev->stats.rx_length_errors;
			atomic_inc((atomic_t *) &ppp->dev->stats.rx_errors);
			kfree_skb(skb);
		}
	}

	ppp_recv_unlock(ppp);
	return;

 err:
	kfree_skb(skb);
	atomic_inc((atomic_t *) &ppp->dev->stats.rx_errors);
}

/*
 * Reconstruct a packet from the MP fragment queue.
 */
static struct sk_buff *
ppp_mp_reconstruct(struct ppp *ppp)
{
	struct sk_buff_head *list = &ppp->mrq;
	struct sk_buff *p, *tmp;
	struct sk_buff *head = NULL;
	struct sk_buff *tail = NULL;
	unsigned long now = jiffies;
	unsigned seq;

	skb_queue_walk_safe(list, p, tmp) {
		if (now - PPP_MP_CB(p)->timestamp >= 10) {
			/* fragment is too old, drop it */
			head = NULL;
			__skb_unlink(p, list);
			kfree_skb(p);

			++ppp->dev->stats.rx_dropped;
			atomic_inc((atomic_t *) &ppp->dev->stats.rx_errors);
			continue;
		}

		/* B bit set indicates this fragment starts a packet */
		if (PPP_MP_CB(p)->BEbits & B) {
			head = p;
			seq = PPP_MP_CB(p)->sequence;
		}
		else if (!head) {
			continue;
			}
		else if (((PPP_MP_CB(p)->sequence - seq) & 0xfff) != 0) {
			head = NULL;
			continue;
		}

		/* Got a complete packet yet? */
		if (!(PPP_MP_CB(p)->BEbits & E)) {
		++seq;
			continue;
	}
		tail = p;

		if (head != tail) {
			struct sk_buff **fragpp = &skb_shinfo(head)->frag_list;
			while (*fragpp) fragpp = &(*fragpp)->next;

			p = skb_queue_next(list, head);
			__skb_unlink(head, list);
			skb_queue_walk_from_safe(list, p, tmp) {
				__skb_unlink(p, list);
				*fragpp = p;
				p->next = NULL;
				fragpp = &p->next;

				head->len += p->len;
				head->data_len += p->len;
				head->truesize += p->len;

				if (p == tail)
					break;
			}
		} else {
			__skb_unlink(head, list);
		}
		return head;
	}
	return NULL;
}
#endif /* CONFIG_PPP_MULTILINK */

/*
 * Channel interface.
 */

/* Create a new, unattached ppp channel. */
int ppp_register_channel(struct ppp_channel *chan)
{
	return ppp_register_net_channel(current->nsproxy->net_ns, chan);
}

/* Create a new, unattached ppp channel for specified net. */
int ppp_register_net_channel(struct net *net, struct ppp_channel *chan)
{
	struct channel *pch;
	struct ppp_net *pn;

	pch = kzalloc(sizeof(struct channel), GFP_KERNEL);
	if (!pch)
		return -ENOMEM;

	pn = ppp_pernet(net);

	rcu_assign_pointer(pch->ppp, NULL);
	rcu_assign_pointer(pch->chan, chan);
	pch->chan_net = net;
	rcu_assign_pointer(chan->ppp, pch);
	init_ppp_file(&pch->file, CHANNEL);
	pch->file.hdrlen = chan->hdrlen;
	init_rwsem(&pch->chan_sem);
	spin_lock_init(&pch->downl);
	rwlock_init(&pch->upl);

	spin_lock_bh(&pn->all_channels_lock);
	pch->file.index = ++pn->last_channel_index;
	list_add(&pch->list, &pn->new_channels);
	atomic_inc(&channel_count);
	spin_unlock_bh(&pn->all_channels_lock);

	return 0;
}

/*
 * Return the index of a channel.
 */
int ppp_channel_index(struct ppp_channel *chan)
{
	struct channel *pch = chan->ppp;

	if (pch)
		return pch->file.index;
	return -1;
}

/*
 * Return the PPP unit number to which a channel is connected.
 */
int ppp_unit_number(struct ppp_channel *chan)
{
	struct channel *pch = chan->ppp;
	int unit = -1;

	if (pch) {
		read_lock_bh(&pch->upl);
		if (pch->ppp)
			unit = pch->ppp->file.index;
		read_unlock_bh(&pch->upl);
	}
	return unit;
}

/*
 * Return the PPP device interface name of a channel.
 */
char *ppp_dev_name(struct ppp_channel *chan)
{
	struct channel *pch = chan->ppp;
	char *name = NULL;

	if (pch) {
		read_lock_bh(&pch->upl);
		if (pch->ppp && pch->ppp->dev)
			name = pch->ppp->dev->name;
		read_unlock_bh(&pch->upl);
	}
	return name;
}


/*
 * Disconnect a channel from the generic layer.
 * This must be called in process context.
 */
void
ppp_unregister_channel(struct ppp_channel *chan)
{
	struct channel *pch = chan->ppp;
	struct ppp_net *pn;

	if (!pch)
		return;		/* should never happen */

	rcu_assign_pointer(chan->ppp, NULL);

	/*
	 * This ensures that we have returned from any calls into the
	 * the channel's start_xmit or ioctl routine before we proceed.
	 */
	down_write(&pch->chan_sem);
	spin_lock_bh(&pch->downl);
	rcu_assign_pointer(pch->chan, NULL);
	spin_unlock_bh(&pch->downl);
	up_write(&pch->chan_sem);
	write_lock_bh(&pch->upl);
	pch->file.dead = 1;
	write_unlock_bh(&pch->upl);
	ppp_disconnect_channel(pch);

	pn = ppp_pernet(pch->chan_net);
	spin_lock_bh(&pn->all_channels_lock);
	list_del(&pch->list);
	spin_unlock_bh(&pn->all_channels_lock);

	wake_up_interruptible(&pch->file.rwait);
	if (atomic_dec_and_test(&pch->file.refcnt))
		ppp_destroy_channel(pch);
}

/*
 * Callback from a channel when it can accept more to transmit.
 * This should be called at BH/softirq level, not interrupt level.
 */
void
ppp_output_wakeup(struct ppp_channel *chan)
{
	struct channel *pch = chan->ppp;

	if (!pch)
		return;
	ppp_channel_push(pch);
}

struct net_device *ppp_get_device(struct ppp_channel *chan) {
	struct channel *ch = chan->ppp;
	if (ch && ch->ppp) return ch->ppp->dev;
	return NULL;
}

/*
 * Compression control.
 */

/* Process the PPPIOCSCOMPRESS ioctl. */
static int
ppp_set_compress(struct ppp *ppp, unsigned long arg)
{
	int err;
	struct compressor *cp, *ocomp;
	struct ppp_option_data data;
	void *state, *ostate;
	unsigned char ccp_option[CCP_MAX_OPTION_LENGTH];

	err = -EFAULT;
	if (copy_from_user(&data, (void __user *) arg, sizeof(data)) ||
	    (data.length <= CCP_MAX_OPTION_LENGTH &&
	     copy_from_user(ccp_option, (void __user *) data.ptr, data.length)))
		goto out;
	err = -EINVAL;
	if (data.length > CCP_MAX_OPTION_LENGTH ||
	    ccp_option[1] < 2 || ccp_option[1] > data.length)
		goto out;

	cp = try_then_request_module(
		find_compressor(ccp_option[0]),
		"ppp-compress-%d", ccp_option[0]);
	if (!cp)
		goto out;

	err = -ENOBUFS;
	if (data.transmit) {
		state = cp->comp_alloc(ccp_option, data.length);
		if (state) {
			ppp_xmit_lock(ppp);
			ppp->xstate &= ~SC_COMP_RUN;
			ocomp = rcu_dereference(ppp->xcomp);
			ostate = rcu_dereference(ppp->xc_state);
			rcu_assign_pointer(ppp->xcomp, cp);
			rcu_assign_pointer(ppp->xc_state, state);
			ppp_xmit_unlock(ppp);
			if (ostate) {
				ocomp->comp_free(ostate);
				module_put(ocomp->owner);
			}
			err = 0;
		} else
			module_put(cp->owner);

	} else {
		state = cp->decomp_alloc(ccp_option, data.length);
		if (state) {
			ppp_recv_lock(ppp);
			ppp->rstate &= ~SC_DECOMP_RUN;
			ocomp = rcu_dereference(ppp->rcomp);
			ostate = rcu_dereference(ppp->rc_state);
			rcu_assign_pointer(ppp->rcomp, cp);
			rcu_assign_pointer(ppp->rc_state, state);
			ppp_recv_unlock(ppp);
			if (ostate) {
				ocomp->decomp_free(ostate);
				module_put(ocomp->owner);
			}
			err = 0;
		} else
			module_put(cp->owner);
	}

 out:
	return err;
}

/*
 * Look at a CCP packet and update our state accordingly.
 * We assume the caller has the xmit or recv path locked.
 */
static void
ppp_ccp_peek(struct ppp *ppp, struct sk_buff *skb, int inbound)
{
	unsigned char *dp;
	int len;

	if (!pskb_may_pull(skb, CCP_HDRLEN + 2))
		return;	/* no header */
	dp = skb->data + 2;

	switch (CCP_CODE(dp)) {
	case CCP_CONFREQ:

		/* A ConfReq starts negotiation of compression
		 * in one direction of transmission,
		 * and hence brings it down...but which way?
		 *
		 * Remember:
		 * A ConfReq indicates what the sender would like to receive
		 */
		if(inbound)
			/* He is proposing what I should send */
			ppp->xstate &= ~SC_COMP_RUN;
		else
			/* I am proposing to what he should send */
			ppp->rstate &= ~SC_DECOMP_RUN;

		break;

	case CCP_TERMREQ:
	case CCP_TERMACK:
		/*
		 * CCP is going down, both directions of transmission
		 */
		ppp->rstate &= ~SC_DECOMP_RUN;
		ppp->xstate &= ~SC_COMP_RUN;
		break;

	case CCP_CONFACK:
		if ((ppp->flags & (SC_CCP_OPEN | SC_CCP_UP)) != SC_CCP_OPEN)
			break;
		len = CCP_LENGTH(dp);
		if (!pskb_may_pull(skb, len + 2))
			return;		/* too short */
		dp += CCP_HDRLEN;
		len -= CCP_HDRLEN;
		if (len < CCP_OPT_MINLEN || len < CCP_OPT_LENGTH(dp))
			break;
		if (inbound) {
			/* we will start receiving compressed packets */
			if (!ppp->rc_state)
				break;
			if (ppp->rcomp->decomp_init(ppp->rc_state, dp, len,
					ppp->file.index, 0, ppp->mru, ppp->debug)) {
				ppp->rstate |= SC_DECOMP_RUN;
				ppp->rstate &= ~(SC_DC_ERROR | SC_DC_FERROR);
			}
		} else {
			/* we will soon start sending compressed packets */
			if (!ppp->xc_state)
				break;
			if (ppp->xcomp->comp_init(ppp->xc_state, dp, len,
					ppp->file.index, 0, ppp->debug))
				ppp->xstate |= SC_COMP_RUN;
		}
		break;

	case CCP_RESETACK:
		/* reset the [de]compressor */
		if ((ppp->flags & SC_CCP_UP) == 0)
			break;
		if (inbound) {
			if (ppp->rc_state && (ppp->rstate & SC_DECOMP_RUN)) {
				ppp->rcomp->decomp_reset(ppp->rc_state);
				ppp->rstate &= ~SC_DC_ERROR;
			}
		} else {
			if (ppp->xc_state && (ppp->xstate & SC_COMP_RUN))
				ppp->xcomp->comp_reset(ppp->xc_state);
		}
		break;
	}
}

/* Free up compression resources. */
static void
ppp_ccp_closed(struct ppp *ppp)
{
	void *xstate, *rstate;
	struct compressor *xcomp, *rcomp;

	ppp_lock(ppp);
	ppp->flags &= ~(SC_CCP_OPEN | SC_CCP_UP);
	ppp->xstate = 0;
	xcomp = rcu_dereference(ppp->xcomp);
	xstate = rcu_dereference(ppp->xc_state);
	rcu_assign_pointer(ppp->xc_state, NULL);
	ppp->rstate = 0;
	rcomp = rcu_dereference(ppp->rcomp);
	rstate = rcu_dereference(ppp->rc_state);
	rcu_assign_pointer(ppp->rc_state, NULL);
	ppp_unlock(ppp);

	if (xstate) {
		xcomp->comp_free(xstate);
		module_put(xcomp->owner);
	}
	if (rstate) {
		rcomp->decomp_free(rstate);
		module_put(rcomp->owner);
	}
}

/* List of compressors. */
static LIST_HEAD(compressor_list);
static DEFINE_SPINLOCK(compressor_list_lock);

struct compressor_entry {
	struct list_head list;
	struct compressor *comp;
};

static struct compressor_entry *
find_comp_entry(int proto)
{
	struct compressor_entry *ce;

	list_for_each_entry(ce, &compressor_list, list) {
		if (ce->comp->compress_proto == proto)
			return ce;
	}
	return NULL;
}

/* Register a compressor */
int
ppp_register_compressor(struct compressor *cp)
{
	struct compressor_entry *ce;
	int ret;
	spin_lock(&compressor_list_lock);
	ret = -EEXIST;
	if (find_comp_entry(cp->compress_proto))
		goto out;
	ret = -ENOMEM;
	ce = kmalloc(sizeof(struct compressor_entry), GFP_ATOMIC);
	if (!ce)
		goto out;
	ret = 0;
	ce->comp = cp;
	list_add(&ce->list, &compressor_list);
 out:
	spin_unlock(&compressor_list_lock);
	return ret;
}

/* Unregister a compressor */
void
ppp_unregister_compressor(struct compressor *cp)
{
	struct compressor_entry *ce;

	spin_lock(&compressor_list_lock);
	ce = find_comp_entry(cp->compress_proto);
	if (ce && ce->comp == cp) {
		list_del(&ce->list);
		kfree(ce);
	}
	spin_unlock(&compressor_list_lock);
}

/* Find a compressor. */
static struct compressor *
find_compressor(int type)
{
	struct compressor_entry *ce;
	struct compressor *cp = NULL;

	spin_lock(&compressor_list_lock);
	ce = find_comp_entry(type);
	if (ce) {
		cp = ce->comp;
		if (!try_module_get(cp->owner))
			cp = NULL;
	}
	spin_unlock(&compressor_list_lock);
	return cp;
}

/*
 * Miscelleneous stuff.
 */

void ppp_get_stats(struct ppp *ppp, struct ppp_stats *st)
{
	memset(st, 0, sizeof(*st));
	st->p.ppp_ipackets = ppp->dev->stats.rx_packets;
	st->p.ppp_ierrors = ppp->dev->stats.rx_errors;
	st->p.ppp_ibytes = ppp->dev->stats.rx_bytes;
	st->p.ppp_opackets = ppp->dev->stats.tx_packets;
	st->p.ppp_oerrors = ppp->dev->stats.tx_errors;
	st->p.ppp_obytes = ppp->dev->stats.tx_bytes;
}
EXPORT_SYMBOL(ppp_get_stats);

/*
 * Stuff for handling the lists of ppp units and channels
 * and for initialization.
 */

/*
 * Create a new ppp interface unit.  Fails if it can't allocate memory
 * or if there is already a unit with the requested number.
 * unit == -1 means allocate a new number.
 */
static struct ppp *
ppp_create_interface(struct net *net, int unit, int *retp)
{
	struct ppp *ppp;
	struct ppp_net *pn;
	struct net_device *dev = NULL;
	int ret = -ENOMEM;
	int i;

	dev = alloc_netdev(sizeof(struct ppp), "", ppp_setup);
	if (!dev)
		goto out1;

	pn = ppp_pernet(net);

	ppp = netdev_priv(dev);
	ppp->dev = dev;
	ppp->mru = PPP_MRU;
	ppp->chan_min_mtu = PPP_MTU;
	init_ppp_file(&ppp->file, INTERFACE);
	ppp->file.hdrlen = PPP_HDRLEN - 2;	/* don't count proto bytes */
	for (i = 0; i < NUM_NP; ++i)
		ppp->npmode[i] = NPMODE_DROP;
	INIT_LIST_HEAD(&ppp->channels);
	spin_lock_init(&ppp->rlock);
	spin_lock_init(&ppp->wlock);
#ifdef CONFIG_PPP_MULTILINK
	skb_queue_head_init(&ppp->mrq);
#endif /* CONFIG_PPP_MULTILINK */

	/*
	 * drum roll: don't forget to set
	 * the net device is belong to
	 */
	dev_net_set(dev, net);

	mutex_lock(&pn->all_ppp_mutex);

	if (unit < 0) {
		unit = unit_get(&pn->units_idr, ppp, pn->last_unit_index++);
		if (unit < 0) {
			ret = unit;
			goto out2;
		}
	} else {
		ret = -EEXIST;
		if (unit_find(&pn->units_idr, unit))
			goto out2; /* unit already exists */
		/*
		 * if caller need a specified unit number
		 * lets try to satisfy him, otherwise --
		 * he should better ask us for new unit number
		 *
		 * NOTE: yes I know that returning EEXIST it's not
		 * fair but at least pppd will ask us to allocate
		 * new unit in this case so user is happy :)
		 */
		unit = unit_set(&pn->units_idr, ppp, unit);
		if (unit < 0)
			goto out2;
	}

	/* Initialize the new ppp unit */
	ppp->file.index = unit;
	sprintf(dev->name, "ppp%d", unit);

#if 0 /* register later */
	ret = register_netdev(dev);
	if (ret != 0) {
		unit_put(&pn->units_idr, unit);
		netdev_err(ppp->dev, "PPP: couldn't register device %s (%d)\n",
			   dev->name, ret);
		goto out2;
	}
#endif

	ppp->ppp_net = net;

	atomic_inc(&ppp_unit_count);
	mutex_unlock(&pn->all_ppp_mutex);

	*retp = 0;
	return ppp;

out2:
	mutex_unlock(&pn->all_ppp_mutex);
	free_netdev(dev);
out1:
	*retp = ret;
	return NULL;
}

/*
 * Initialize a ppp_file structure.
 */
static void
init_ppp_file(struct ppp_file *pf, int kind)
{
	pf->kind = kind;
	skb_queue_head_init(&pf->xq);
	skb_queue_head_init(&pf->rq);
	atomic_set(&pf->refcnt, 1);
	init_waitqueue_head(&pf->rwait);
}

/*
 * Take down a ppp interface unit - called when the owning file
 * (the one that created the unit) is closed or detached.
 */
static void ppp_shutdown_interface(struct ppp *ppp)
{
	struct ppp_net *pn;

	pn = ppp_pernet(ppp->ppp_net);
	mutex_lock(&pn->all_ppp_mutex);

	/* This will call dev_close() for us. */
	ppp_lock(ppp);
	if (!ppp->closing) {
		ppp->closing = 1;
		ppp_unlock(ppp);
		rtnl_lock();
		if (ppp->dev->reg_state == NETREG_REGISTERED) {
			if (list_empty(&pn->unreg_list))
				schedule_delayed_work(&pn->unreg_work, HZ);
			atomic_inc(&ppp->file.refcnt);
			dev_close(ppp->dev);
			unregister_netdevice_queue(ppp->dev, &pn->unreg_list);
		}
		rtnl_unlock();
		unit_put(&pn->units_idr, ppp->file.index);
	} else {
		ppp_unlock(ppp);
	}

	ppp->file.dead = 1;
	ppp->owner = NULL;
	wake_up_interruptible(&ppp->file.rwait);

	mutex_unlock(&pn->all_ppp_mutex);
}

/*
 * Free the memory used by a ppp unit.  This is only called once
 * there are no channels connected to the unit and no file structs
 * that reference the unit.
 */
static void ppp_destroy_interface(struct ppp *ppp)
{
	atomic_dec(&ppp_unit_count);

	if (!ppp->file.dead || ppp->n_channels) {
		/* "can't happen" */
		printk("ppp: destroying ppp struct %p "
			   "but dead=%d n_channels=%d !\n",
			   ppp, ppp->file.dead, ppp->n_channels);
		BUG();
		return;
	}

	ppp_ccp_closed(ppp);
	skb_queue_purge(&ppp->file.xq);
	skb_queue_purge(&ppp->file.rq);
#ifdef CONFIG_PPP_MULTILINK
	skb_queue_purge(&ppp->mrq);
#endif /* CONFIG_PPP_MULTILINK */
#ifdef CONFIG_PPP_FILTER
	kfree(ppp->pass_filter);
	ppp->pass_filter = NULL;
	kfree(ppp->active_filter);
	ppp->active_filter = NULL;
#endif /* CONFIG_PPP_FILTER */
	free_netdev(ppp->dev);
}

/*
 * Locate an existing ppp unit.
 * The caller should have locked the all_ppp_mutex.
 */
static struct ppp *
ppp_find_unit(struct ppp_net *pn, int unit)
{
	return unit_find(&pn->units_idr, unit);
}

/*
 * Locate an existing ppp channel.
 * The caller should have locked the all_channels_lock.
 * First we look in the new_channels list, then in the
 * all_channels list.  If found in the new_channels list,
 * we move it to the all_channels list.  This is for speed
 * when we have a lot of channels in use.
 */
static struct channel *
ppp_find_channel(struct ppp_net *pn, int unit)
{
	struct channel *pch;

	list_for_each_entry(pch, &pn->new_channels, list) {
		if (pch->file.index == unit) {
			list_move(&pch->list, &pn->all_channels);
			return pch;
		}
	}

	list_for_each_entry(pch, &pn->all_channels, list) {
		if (pch->file.index == unit)
			return pch;
	}

	return NULL;
}

/*
 * Connect a PPP channel to a PPP interface unit.
 */
static int
ppp_connect_channel(struct channel *pch, int unit)
{
	struct ppp *ppp;
	struct ppp_net *pn;
	int ret = -ENXIO;
	int hdrlen;

	pn = ppp_pernet(pch->chan_net);

	mutex_lock(&pn->all_ppp_mutex);
	ppp = ppp_find_unit(pn, unit);
	if (!ppp)
		goto out;
	write_lock_bh(&pch->upl);
	ret = -EINVAL;
	if (pch->ppp)
		goto outl;
	if (pch->file.dead)
		goto outl;

	ppp_lock(ppp);
	if (pch->file.hdrlen > ppp->file.hdrlen)
		ppp->file.hdrlen = pch->file.hdrlen;
	hdrlen = pch->file.hdrlen + 2;	/* for protocol bytes */
	if (hdrlen > ppp->dev->hard_header_len)
		ppp->dev->hard_header_len = hdrlen;
	list_add_tail_rcu(&pch->clist, &ppp->channels);
	++ppp->n_channels;
	rcu_assign_pointer(pch->ppp, ppp);
	atomic_inc(&ppp->file.refcnt);
	if (pch->chan->mtu && ppp->chan_min_mtu > pch->chan->mtu)
		ppp->chan_min_mtu = pch->chan->mtu;
	if (pch->chan->ops->attached)
		pch->chan->ops->attached(pch->chan);
	ppp_unlock(ppp);
	ret = 0;

 outl:
	write_unlock_bh(&pch->upl);
 out:
	mutex_unlock(&pn->all_ppp_mutex);
	return ret;
}

/*
 * Disconnect a channel from its ppp unit.
 */
static int
ppp_disconnect_channel(struct channel *pch)
{
	struct ppp *ppp;
	int err = -EINVAL;

	write_lock_bh(&pch->upl);
	ppp = pch->ppp;
	rcu_assign_pointer(pch->ppp, NULL);
	write_unlock_bh(&pch->upl);
	if (ppp) {
		/* remove it from the ppp unit's list */
		ppp_lock(ppp);
		list_del_rcu(&pch->clist);
		if (--ppp->n_channels == 0)
			wake_up_interruptible(&ppp->file.rwait);
		ppp_unlock(ppp);
		if (atomic_dec_and_test(&ppp->file.refcnt))
			ppp_destroy_interface(ppp);
		err = 0;
	}
	return err;
}

/*
 * Free up the resources used by a ppp channel.
 */
static void ppp_destroy_channel(struct channel *pch)
{
	atomic_dec(&channel_count);

	if (!pch->file.dead) {
		/* "can't happen" */
		pr_err("ppp: destroying undead channel %p !\n", pch);
		return;
	}
	skb_queue_purge(&pch->file.xq);
	skb_queue_purge(&pch->file.rq);
	kfree_rcu(pch, rcu);
}

static void __exit ppp_cleanup(void)
{
	/* should never happen */
	if (atomic_read(&ppp_unit_count) || atomic_read(&channel_count))
		pr_err("PPP: removing module but units remain!\n");
	unregister_chrdev(PPP_MAJOR, "ppp");
	device_destroy(ppp_class, MKDEV(PPP_MAJOR, 0));
	class_destroy(ppp_class);
	unregister_pernet_device(&ppp_net_ops);
}

/*
 * Units handling. Caller must protect concurrent access
 * by holding all_ppp_mutex
 */

static int __unit_alloc(struct idr *p, void *ptr, int n)
{
	int unit, err;

again:
	if (!idr_pre_get(p, GFP_KERNEL)) {
		pr_err("PPP: No free memory for idr\n");
		return -ENOMEM;
	}

	err = idr_get_new_above(p, ptr, n, &unit);
	if (err < 0) {
		if (err == -EAGAIN)
			goto again;
		return err;
	}

	return unit;
}

/* associate pointer with specified number */
static int unit_set(struct idr *p, void *ptr, int n)
{
	int unit;

	unit = __unit_alloc(p, ptr, n);
	if (unit < 0)
		return unit;
	else if (unit != n) {
		idr_remove(p, unit);
		return -EINVAL;
	}

	return unit;
}

/* get new free unit number and associate pointer with it */
static int unit_get(struct idr *p, void *ptr, int n)
{
	return __unit_alloc(p, ptr, n);
}

/* put unit number back to a pool */
static void unit_put(struct idr *p, int n)
{
	idr_remove(p, n);
}

/* get pointer associated with the number */
static void *unit_find(struct idr *p, int n)
{
	return idr_find(p, n);
}

/* Module/initialization stuff */

module_init(ppp_init);
module_exit(ppp_cleanup);

EXPORT_SYMBOL(ppp_register_net_channel);
EXPORT_SYMBOL(ppp_register_channel);
EXPORT_SYMBOL(ppp_unregister_channel);
EXPORT_SYMBOL(ppp_channel_index);
EXPORT_SYMBOL(ppp_unit_number);
EXPORT_SYMBOL(ppp_dev_name);
EXPORT_SYMBOL(ppp_input);
EXPORT_SYMBOL(ppp_input_error);
EXPORT_SYMBOL(ppp_output_wakeup);
EXPORT_SYMBOL(ppp_register_compressor);
EXPORT_SYMBOL(ppp_unregister_compressor);
EXPORT_SYMBOL(ppp_get_device);
MODULE_LICENSE("GPL");
MODULE_ALIAS_CHARDEV(PPP_MAJOR, 0);
MODULE_ALIAS("devname:ppp");
