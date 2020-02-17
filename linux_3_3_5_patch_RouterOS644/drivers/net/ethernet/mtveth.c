#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <asm/vm.h>

#define MAX_IFACES	8

#define TXBUF_COUNT	1024
#define RXBUF_COUNT	128

#define RXBUF_SIZE	1600

extern int vm_create_queue(unsigned id, unsigned irq,
			   unsigned tx, unsigned rx);
extern int vm_release_queue(unsigned id);

#define CMD_NEWIFACE	0
#define CMD_DELIFACE	1

struct ctrl_msg {
	unsigned cmd;
	unsigned short id;
	unsigned char hwaddr[6];
} __attribute__((packed));

static volatile struct vdma_descr rx_descr[RXBUF_COUNT];
static volatile struct vdma_descr tx_descr[TXBUF_COUNT];
static struct sk_buff *rx_skbs[RXBUF_COUNT];
static struct sk_buff *tx_skbs[TXBUF_COUNT];

static unsigned last_tx;
static atomic_t cur_tx;
static unsigned cur_rx;
static unsigned max_tx;

static struct net_device *devs[MAX_IFACES];

struct veth_private {
	unsigned id;
	atomic_t pending_tx;
};

static void ctrl_receiver(struct work_struct *work);

static struct sk_buff_head ctrl_queue;
static DECLARE_WORK(ctrl_work, ctrl_receiver);

static int veth_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct veth_private *veth = netdev_priv(dev);
	unsigned cur = atomic_read(&cur_tx) % TXBUF_COUNT;
    
	if (skb_padto(skb, ETH_ZLEN))
		return NETDEV_TX_OK;

	if (tx_descr[cur].size & DONE) {
		dev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	if (skb_headroom(skb) < 2) {
		struct sk_buff *s = skb;
		skb = skb_realloc_headroom(s, 2);
		dev_kfree_skb_any(s);
	} else {
		skb = skb_unshare(skb, GFP_ATOMIC);
	}
	if (!skb) {
		dev->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}
	*(u16 *) skb_push(skb, 2) = veth->id;

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	tx_descr[cur].addr = (unsigned) skb->data;
	tx_descr[cur].size = skb->len | DONE;

	if (tx_skbs[cur]) {
		/* should not happen */
		dev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_BUSY;
	}

	tx_skbs[cur] = skb;
	atomic_add(1, &cur_tx);

	if (atomic_add_return(1, &veth->pending_tx) >= max_tx) {
		netif_stop_queue(dev);

		/* in case we got rewaken right before stop */
		if (atomic_read(&veth->pending_tx) < max_tx)
			netif_wake_queue(dev);
	}

	return 0;
}

static irqreturn_t veth_interrupt(int irq, void *dev_id)
{
	unsigned cur;

	while (last_tx != atomic_read(&cur_tx)) {
		unsigned last = last_tx % TXBUF_COUNT;
		struct net_device *dev;
		struct veth_private *veth;

		if (tx_descr[last].size & DONE)
			break;

		dev = tx_skbs[last]->dev;
		veth = netdev_priv(dev);
		dev_kfree_skb_irq(tx_skbs[last]);
		tx_skbs[last] = NULL;

		++last_tx;

		if (atomic_sub_return(1, &veth->pending_tx) < max_tx)
			netif_wake_queue(dev);
	}

	cur = cur_rx % RXBUF_COUNT;
	while ((rx_descr[cur].size & DONE)) {
		struct sk_buff *skb = rx_skbs[cur];
		struct net_device *dev;
		unsigned id;

		skb_put(skb, rx_descr[cur].size & ~DONE);
		if (skb->len < 2) {
			dev_kfree_skb_irq(skb);
			goto next;
		}

		id = *(u16 *) skb->data;
		skb_pull(skb, 2);

		if (id == 0) {
			__skb_queue_tail(&ctrl_queue, skb);
			schedule_work(&ctrl_work);
			goto next;
		}
		if (id >= MAX_IFACES || !devs[id]) {
			dev_kfree_skb_irq(skb);
			goto next;
		}
		dev = devs[id];

		skb->dev = dev;
		skb->protocol = eth_type_trans(skb, dev);

		dev->last_rx = jiffies;
		++dev->stats.rx_packets;
		dev->stats.rx_bytes += skb->len;

		netif_rx(skb);

	  next:
		skb = dev_alloc_skb(RXBUF_SIZE);
		rx_skbs[cur] = skb;
		if (skb) {
			rx_descr[cur].addr = (unsigned) skb->data;
			rx_descr[cur].size = RXBUF_SIZE;
		} else {
			rx_descr[cur].size = 0;
		}

		++cur_rx;
		cur = cur_rx % RXBUF_COUNT;
	}

	return IRQ_HANDLED;
}

static const struct net_device_ops veth_netdev_ops = {
	.ndo_start_xmit		= veth_xmit,
};

static int veth_alloc_dev(unsigned id, const unsigned char *hwaddr)
{
	struct veth_private *veth;
	struct net_device *dev;
	int err;

	//SET_NETDEV_DEV(dev, &pdev->dev);
	//platform_set_drvdata(pdev, dev);

	dev = alloc_etherdev(sizeof(struct veth_private));
	if (!dev)
		return -ENOMEM;

	veth = netdev_priv(dev);
	veth->id = id;
	atomic_set(&veth->pending_tx, 1);
	memcpy(dev->dev_addr, hwaddr, 6);
	dev->netdev_ops = &veth_netdev_ops;

	err = register_netdev(dev);
	if (err < 0) {
		printk("cannot register net device %u\n", err);
		goto netdev_err;
	}

	devs[id] = dev;
	return 0;

  netdev_err:
	free_netdev(dev);
	return err;
}

static int recv_ctrl_msg(struct sk_buff *skb)
{
	struct ctrl_msg *msg = (struct ctrl_msg *) skb->data;

	if (skb->len < sizeof(struct ctrl_msg))
		return -EINVAL;

	if (msg->cmd == CMD_NEWIFACE) {
		if (msg->id >= MAX_IFACES || devs[msg->id])
			return -EBUSY;

		veth_alloc_dev(msg->id, msg->hwaddr);
		return 0;
	} else if (msg->cmd == CMD_DELIFACE) {
		struct net_device *dev;

		if (msg->id >= MAX_IFACES || !devs[msg->id])
			return -EINVAL;
		
		dev = devs[msg->id];
		devs[msg->id] = NULL;

		unregister_netdev(dev);
	}
	return -EINVAL;
}

static void ctrl_receiver(struct work_struct *work)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&ctrl_queue)))
		recv_ctrl_msg(skb);
}

int veth_init(void)
{
	unsigned i;

	if (vm_running() != 0)
		return 0;

	skb_queue_head_init(&ctrl_queue);

	if (request_irq(get_virq_nr(3), veth_interrupt, IRQF_SHARED,
			"veth", (void *) 1))
		return -EBUSY;

	for (i = 0; i < TXBUF_COUNT; ++i) {
		tx_descr[i].addr = 0;
		tx_descr[i].size = 0;
		tx_descr[i].next = (unsigned) &tx_descr[i + 1];
	}
	for (i = 0; i < RXBUF_COUNT; ++i) {
		rx_skbs[i] = dev_alloc_skb(RXBUF_SIZE);
		rx_descr[i].addr = (unsigned) rx_skbs[i]->data;
		rx_descr[i].size = RXBUF_SIZE;
		rx_descr[i].next = (unsigned) &rx_descr[i + 1];
	}
	tx_descr[TXBUF_COUNT - 1].next = (unsigned) &tx_descr[0];
	rx_descr[RXBUF_COUNT - 1].next = (unsigned) &rx_descr[0];
	
	vm_create_queue(3, 3,
			(unsigned) &tx_descr[0], (unsigned) &rx_descr[0]);

	max_tx = TXBUF_COUNT / MAX_IFACES;

	return 0;
}
module_init(veth_init);
