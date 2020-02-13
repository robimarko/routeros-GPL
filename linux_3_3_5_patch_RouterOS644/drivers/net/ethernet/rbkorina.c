/*
        Copyright 2003 Integrated Device Technology, Inc.
        Copyright 2002 MontaVista Software Inc.
                stevel@mvista.com or source@mvista.com
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/slab.h> 
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <asm/unaligned.h>

#define KORINA_DMA_RX_IRQ 40
#define KORINA_DMA_TX_IRQ 41
#define KORINA_UNDERFLOW_IRQ 114

#define TX_RING_SIZE	128
#define TX_QUEUE_LEN	120
#define RX_RING_SIZE	128

#define PKT_BUF_SZ	1636


#define KORINA_BASE 0x18060000
#define DMA_BASE 0x18040000

#define DMA_REG(channel, reg) \
        ((*(volatile u32 *) KSEG1ADDR(DMA_BASE + (channel) * 20 + (reg))))

#define KORINA_REG(reg) \
        ((*(volatile u32 *) KSEG1ADDR(KORINA_BASE + (reg))))

#define DMA_RX_CHANNEL 0
#define DMA_TX_CHANNEL 1

#define DMA_CONTROLL    0x00
#define DMA_STATUS      0x04
#define DMA_STATUS_MASK 0x08
#define DMA_DESCR_PTR   0x0c
#define DMA_NEXT_DESCR  0x10

#define DMAC_RUN        0x01
#define DMAC_DONE       0x02
#define DMAC_ABORT      0x10

#define DMAS_FINISHED	0x01
#define DMAS_DONE	0x02
#define DMAS_ERROR	0x08
#define DMAS_HALT       0x10

#define DMASM_FINISHED	0x01
#define DMASM_DONE	0x02
#define DMASM_CHAIN	0x04
#define DMASM_ERROR	0x08
#define DMASM_HALT	0x10

#define KORINA_CONTROL		0x0000
#define KORINA_FIFO_TX_THRE	0x0004
#define KORINA_ADDR_RECOGN	0x0008
#define KORINA_CLOCK		0x0028
#define KORINA_ADDR_LO		0x0100
#define KORINA_ADDR_HI		0x0104
#define KORINA_CONFIG1		0x0200
#define KORINA_CONFIG2		0x0204
#define KORINA_BTB_IPACK_GAP	0x0208
#define KORINA_NBTB_IPACK_GAP	0x020c
#define KORINA_MAX_FRAME	0x0214
#define KORINA_MII_CONFIG	0x0220
#define KORINA_MII_COMMAND	0x0224
#define KORINA_MII_ADDR		0x0228
#define KORINA_MII_WRITE_DATA	0x022c
#define KORINA_MII_READ_DATA	0x0230
#define KORINA_MII_IND		0x0234

#define ETHINTFC_ENABLE		0x01
#define ETHINTFC_RESETTING	0x04
#define ETHINTFC_UNDERFLOW	0x20

#define ETHARC_PROMISC		0x01
#define ETHARC_ALL_MULTICAST	0x02
#define ETHARC_BROADCAST	0x08

#define ETHMAC1_RX_ENABLE 0x01

#define ETHMAC2_FULL_DUPLEX	0x01
#define ETHMAC2_CRC_ENABLE	0x10
#define ETHMAC2_PAD_ENABLE	0x20

#define MIIMCFG_RESET		0x8000

#define MIIMCMD_READ		0x01

#define MIIMIND_BUSY		0x01
#define MIIMIND_NOT_VALID	0x04

#define MII_CLOCK 1250000

#define DMAD_COUNT		0x0003ffff
#define DMAD_INT_ON_FINISHED	0x08000000
#define DMAD_INT_ON_DONE	0x10000000
#define DMAD_DONE		0x40000000
#define DMAD_FINISHED		0x80000000

#define DMA_DESCR(base, off) ((base) + sizeof(struct dma_desc) * (off))

#define DEVCS_RX_OK		0x00000004
#define DEVCS_RX_OVERFLOW	0x00000100
#define DEVCS_RX_CRC_ERR	0x00000200
#define DEVCS_RX_CODE_VIOLATION 0x00000400
#define DEVCS_RX_LEN_ERROR	0x00001000
#define DEVCS_RX_BAD_LENGTH	0x00002000

#define DEVCS_TX_FIRST_DESC		0x00000001
#define DEVCS_TX_LAST_DESC		0x00000002
#define DEVCS_TX_OK			0x00000040
#define DEVCS_TX_UNDERFLOW		0x00000200
#define DEVCS_TX_OVERSIZED		0x00000400
#define DEVCS_TX_EXCESSIVE_DEFERAL	0x00000800
#define DEVCS_TX_EXCESSIVE_COLLISIONS	0x00001000
#define DEVCS_TX_LATE_COLLISION		0x00002000


extern unsigned long mips_hpt_frequency;
extern unsigned char mips_mac_address[6];

struct dma_desc {
        u32 control;
        u32 addr;
        u32 devcs;
        u32 link;
};

struct korina_private {
	volatile struct dma_desc *tx_ring;
	volatile struct dma_desc *rx_ring;
	dma_addr_t td_ring_dma;
	dma_addr_t rd_ring_dma;

	struct sk_buff *tx_skb[TX_RING_SIZE];
	struct sk_buff *rx_skb[RX_RING_SIZE];
	dma_addr_t rx_skb_dma[RX_RING_SIZE];

	unsigned cur_rx;
	unsigned dirty_rx;

	unsigned cur_tx;
	unsigned dirty_tx;
	
	spinlock_t lock;

	struct net_device *dev;
	struct mii_if_info mii_if;
	struct napi_struct napi;
};

static void korina_dump(struct net_device *dev) {
    struct korina_private *np = netdev_priv(dev);
    int i;


    printk("TX %d(%d) %d(%d)\n",
	   np->cur_tx, np->cur_tx % TX_RING_SIZE,
	   np->dirty_tx, np->dirty_tx % TX_RING_SIZE);
    printk("DMA_CONTROLL %08x\n", DMA_REG(DMA_TX_CHANNEL, DMA_CONTROLL));
    printk("DMA_STATUS   %08x\n", DMA_REG(DMA_TX_CHANNEL, DMA_STATUS));
    printk("DMA_MASK     %08x\n", DMA_REG(DMA_TX_CHANNEL, DMA_STATUS_MASK));
    printk("DMA_DESCR    %08x\n", DMA_REG(DMA_TX_CHANNEL, DMA_DESCR_PTR));
    printk("DMA_NEXT     %08x\n", DMA_REG(DMA_TX_CHANNEL, DMA_NEXT_DESCR));
    for (i = 0; i < TX_RING_SIZE; ++i) {
	printk("%03d: %08x %08x %08x %08x\n", i,
	       np->tx_ring[i].control,
	       np->tx_ring[i].addr,
	       np->tx_ring[i].devcs,
	       np->tx_ring[i].link
	    );
    }
    printk("RX %d(%d) %d(%d)\n",
	   np->cur_rx, np->cur_rx % RX_RING_SIZE,
	   np->dirty_rx, np->dirty_rx % RX_RING_SIZE);
    printk("DMA_CONTROLL %08x\n", DMA_REG(DMA_RX_CHANNEL, DMA_CONTROLL));
    printk("DMA_STATUS   %08x\n", DMA_REG(DMA_RX_CHANNEL, DMA_STATUS));
    printk("DMA_MASK     %08x\n", DMA_REG(DMA_RX_CHANNEL, DMA_STATUS_MASK));
    printk("DMA_DESCR    %08x\n", DMA_REG(DMA_RX_CHANNEL, DMA_DESCR_PTR));
    printk("DMA_NEXT     %08x\n", DMA_REG(DMA_RX_CHANNEL, DMA_NEXT_DESCR));
    for (i = 0; i < RX_RING_SIZE; ++i) {
	printk("%03d: %08x %08x %08x %08x\n", i,
	       np->rx_ring[i].control,
	       np->rx_ring[i].addr,
	       np->rx_ring[i].devcs,
	       np->rx_ring[i].link
	    );
    }
}

static inline void dma_abort(struct net_device *dev, int channel)
{
        if (DMA_REG(channel, DMA_CONTROLL) & DMAC_RUN) {
                DMA_REG(channel, DMA_CONTROLL) = DMAC_ABORT;
                while (!(DMA_REG(channel, DMA_STATUS) & DMAS_HALT)) {
			dev->trans_start = jiffies;
                }
                DMA_REG(channel, DMA_STATUS) = 0;
        }
        DMA_REG(channel, DMA_DESCR_PTR) = 0;
        DMA_REG(channel, DMA_NEXT_DESCR) = 0;
}

static int mdio_read(struct net_device *dev, int phy_id, int regnum)
{
	int value;

	KORINA_REG(KORINA_MII_ADDR) = (phy_id << 8) | regnum;
	KORINA_REG(KORINA_MII_COMMAND) = MIIMCMD_READ;

	while (KORINA_REG(KORINA_MII_IND) & MIIMIND_BUSY);

	if (KORINA_REG(KORINA_MII_IND) & MIIMIND_NOT_VALID) return 0;

	value = KORINA_REG(KORINA_MII_READ_DATA);
	KORINA_REG(KORINA_MII_COMMAND) = 0;

	return value;
}

static void mdio_write(struct net_device *dev,
		       int phy_id, int regnum, int value)
{
	KORINA_REG(KORINA_MII_ADDR) = (phy_id << 8) | regnum;
	KORINA_REG(KORINA_MII_WRITE_DATA) = value;

	while(KORINA_REG(KORINA_MII_IND) & MIIMIND_BUSY);
}


static void korina_get_drvinfo (struct net_device *dev,
				struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "korina");
	strcpy(info->version, "1.0");
	strcpy(info->bus_info, "00:00.0 korina");
}

static int korina_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct korina_private *np = netdev_priv(dev);

	return mii_ethtool_gset(&np->mii_if, cmd);
}

static int korina_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct korina_private *np = netdev_priv(dev);
	int rc;

	spin_lock_irq(&np->lock);
	rc = mii_ethtool_sset(&np->mii_if, cmd);
	spin_unlock_irq(&np->lock);

	return rc;
}

static int korina_nway_reset(struct net_device *dev)
{
	struct korina_private *np = netdev_priv(dev);

	return mii_nway_restart(&np->mii_if);
}

static u32 korina_get_link(struct net_device *dev)
{
	struct korina_private *np = netdev_priv(dev);

	return mii_link_ok(&np->mii_if);
}

static void korina_multicast_list(struct net_device *dev)
{   
	unsigned recognise = ETHARC_BROADCAST;

	if (dev->flags & IFF_PROMISC)
		recognise |= ETHARC_PROMISC;
	if ((dev->flags & IFF_ALLMULTI) || (netdev_mc_count(dev) > 0))
		recognise |= ETHARC_ALL_MULTICAST;

	KORINA_REG(KORINA_ADDR_RECOGN) = recognise;
}


static struct ethtool_ops korina_ethtool_ops = {
	.get_drvinfo		= korina_get_drvinfo,
	.get_settings		= korina_get_settings,
	.set_settings		= korina_set_settings,
	.nway_reset		= korina_nway_reset,
	.get_link		= korina_get_link,
};

static int korina_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct korina_private *lp = netdev_priv(dev);
	int rc;

#if 0
	if (!netif_running(dev))
		return -EINVAL;
#endif

	spin_lock_irq(&lp->lock);
	rc = generic_mii_ioctl(
		&lp->mii_if, (struct mii_ioctl_data *) &rq->ifr_data, cmd, NULL);
	spin_unlock_irq(&lp->lock);

	return rc;
}

static inline void korina_start_rx(struct korina_private *lp, dma_addr_t rd)
{
        DMA_REG(DMA_RX_CHANNEL, DMA_NEXT_DESCR) = 0;
        DMA_REG(DMA_RX_CHANNEL, DMA_DESCR_PTR) = 0;
        DMA_REG(DMA_RX_CHANNEL, DMA_DESCR_PTR) = rd;
}

static int korina_xmit(struct sk_buff *skb, struct net_device *dev)
{
 	struct korina_private *lp = netdev_priv(dev);
	unsigned int entry = lp->cur_tx % TX_RING_SIZE;
	unsigned int prev_entry = (lp->cur_tx - 1) % TX_RING_SIZE;
 	volatile struct dma_desc *td;

	if (skb_padto(skb, ETH_ZLEN))
		return 0;
    
	spin_lock(&lp->lock);

	lp->tx_skb[entry] = skb;
	td = &lp->tx_ring[entry];
	td->link = 0;
	td->addr = dma_map_single(NULL, skb->data, skb->len, DMA_TO_DEVICE);
	td->devcs = DEVCS_TX_FIRST_DESC | DEVCS_TX_LAST_DESC;
	td->control = skb->len | DMAD_INT_ON_FINISHED;

	lp->tx_ring[prev_entry].link = DMA_DESCR(lp->td_ring_dma, entry);

	if (DMA_REG(DMA_TX_CHANNEL, DMA_NEXT_DESCR) == 0) {
		DMA_REG(DMA_TX_CHANNEL, DMA_NEXT_DESCR) =
			DMA_DESCR(lp->td_ring_dma, entry);
	}

	++lp->cur_tx;
	if (lp->cur_tx == lp->dirty_tx + TX_QUEUE_LEN) {
//		printk("stop queue\n");
//		korina_dump(dev);
		netif_stop_queue(dev);
	}

	dev->trans_start = jiffies;

     	spin_unlock(&lp->lock);

    	return 0;
}

static int korina_rx(struct net_device *dev, int budget);
static void korina_tx(struct net_device *dev);

static int korina_poll(struct napi_struct *napi, int budget)
{
	struct korina_private *lp = container_of(napi, struct korina_private,
						 napi);
	unsigned long flags;
	int work_done;

	DMA_REG(DMA_TX_CHANNEL, DMA_STATUS) = ~(DMASM_FINISHED | DMASM_ERROR);
	DMA_REG(DMA_RX_CHANNEL, DMA_STATUS) = ~(DMASM_DONE | DMASM_ERROR);

	spin_lock(&lp->lock);
	korina_tx(lp->dev);
   	spin_unlock(&lp->lock);

	work_done = korina_rx(lp->dev, budget);

	if (work_done >= budget)
	    return work_done;

	if (DMA_REG(DMA_TX_CHANNEL, DMA_STATUS)
	    & (DMASM_FINISHED | DMASM_ERROR))
	    return budget;
	if (DMA_REG(DMA_RX_CHANNEL, DMA_STATUS)
	    & (DMASM_DONE | DMASM_HALT | DMASM_ERROR))
	    return budget;

	napi_complete(napi);

	local_irq_save(flags);
	DMA_REG(DMA_RX_CHANNEL, DMA_STATUS_MASK) &=
	    ~(DMASM_DONE | DMASM_HALT | DMASM_ERROR);
	DMA_REG(DMA_TX_CHANNEL, DMA_STATUS_MASK) &=
	    ~(DMASM_FINISHED | DMASM_ERROR);
	local_irq_restore(flags);

	return work_done;
}

static irqreturn_t korina_dma_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	struct korina_private *lp = netdev_priv(dev);

	spin_lock(&lp->lock);

	DMA_REG(DMA_RX_CHANNEL, DMA_STATUS_MASK) |=
		DMASM_DONE | DMASM_HALT | DMASM_ERROR;
	DMA_REG(DMA_TX_CHANNEL, DMA_STATUS_MASK) |=
		DMASM_FINISHED | DMASM_ERROR;

	napi_schedule(&lp->napi);

	spin_unlock(&lp->lock);

	return IRQ_HANDLED;
}


static int korina_rx(struct net_device *dev, int budget)
{
	struct korina_private* lp = netdev_priv(dev);
	int boguscnt = lp->dirty_rx + RX_RING_SIZE - lp->cur_rx;
	int cnt = 0;

	/* keep going while we have received into more descriptors */
	while (budget--) {
		unsigned entry = lp->cur_rx % RX_RING_SIZE;
		volatile struct dma_desc *rd = &lp->rx_ring[entry];
		unsigned pktlen;
		unsigned devcs;

		if (!(rd->control & DMAD_DONE)) break;
		if (--boguscnt < 0) break;

	     	devcs = rd->devcs;
	     	pktlen = devcs >> 16;

	     	if (pktlen != PKT_BUF_SZ - (rd->control & DMAD_COUNT)) {
			/*
			 * Due to a bug in korina processor, the packet length
			 * given by devcs field and count field sometimes differ.
			 * If that is the case, report Error.
			 */
			lp->dev->stats.rx_errors++;
			lp->dev->stats.rx_dropped++;
	      	} else if (pktlen < 64 || pktlen > PKT_BUF_SZ) {
			lp->dev->stats.rx_errors++;
			lp->dev->stats.rx_dropped++;
	    	} else if (devcs & DEVCS_RX_OK) {
			struct sk_buff *skb = lp->rx_skb[entry];
			lp->rx_skb[entry] = NULL;
			dma_unmap_single(NULL, lp->rx_skb_dma[entry], pktlen,
					 DMA_FROM_DEVICE);

			skb_put(skb, pktlen - 4);
			skb->protocol = eth_type_trans(skb, dev);
			netif_receive_skb(skb);

			dev->last_rx = jiffies;
			lp->dev->stats.rx_packets++;
			lp->dev->stats.rx_bytes += skb->len;
		} else {
		 	/* This should only happen if we enable 
			   accepting broken packets */
			lp->dev->stats.rx_errors++;
			lp->dev->stats.rx_dropped++;

			if (devcs & DEVCS_RX_CRC_ERR) {
		        	lp->dev->stats.rx_crc_errors++;
		     	} else if (devcs & (DEVCS_RX_BAD_LENGTH
					    | DEVCS_RX_LEN_ERROR)) {
		          	lp->dev->stats.rx_length_errors++;
		      	} else if (devcs & DEVCS_RX_OVERFLOW) {
				/*
				 * The overflow errors are handled through
				 * an interrupt handler.
				 */
				lp->dev->stats.rx_over_errors++;
		     	}
			else if (devcs & DEVCS_RX_CODE_VIOLATION) {
				lp->dev->stats.rx_frame_errors++;
			}
		}
		++lp->cur_rx;
		++cnt;
	}

	/* refill the rx ring buffers */
	for (; lp->cur_rx - lp->dirty_rx > 0; ++lp->dirty_rx) {
		unsigned entry = lp->dirty_rx % RX_RING_SIZE;
		volatile struct dma_desc *rd = &lp->rx_ring[entry];
		struct sk_buff *skb;

		if (lp->rx_skb[entry] == NULL) {
			skb = dev_alloc_skb(PKT_BUF_SZ + 2);
			lp->rx_skb[entry] = skb;
			if (skb == NULL) break;

			skb->dev = dev;
			skb_reserve(skb, 2);
			lp->rx_skb_dma[entry] = dma_map_single(
				NULL, skb->data, PKT_BUF_SZ + 2, DMA_FROM_DEVICE);
		}
		lp->rx_ring[(lp->dirty_rx - 1) % RX_RING_SIZE].link =
			DMA_DESCR(lp->rd_ring_dma, entry);
		rd->control = PKT_BUF_SZ | DMAD_INT_ON_DONE;
		rd->addr = lp->rx_skb_dma[entry];
		rd->devcs = 0;
		rd->link = 0;
	}
	lp->rx_ring[(lp->dirty_rx - 1) % RX_RING_SIZE].link = 0;

	if (DMA_REG(DMA_RX_CHANNEL, DMA_STATUS) & DMAS_HALT) {
		int pos = DMA_REG(DMA_RX_CHANNEL, DMA_DESCR_PTR) -
		    lp->rd_ring_dma;
		DMA_REG(DMA_RX_CHANNEL, DMA_STATUS) = ~DMAS_HALT;

		pos /= sizeof(struct dma_desc);
		++pos;
		pos %= RX_RING_SIZE;
		korina_start_rx(lp, DMA_DESCR(lp->rd_ring_dma, pos));
	}

	return cnt;
}

static void korina_tx(struct net_device *dev)
{
	struct korina_private* lp = netdev_priv(dev);
	u32 devcs;
	int stopped = netif_queue_stopped(dev);

	while (lp->dirty_tx != lp->cur_tx) {
		unsigned entry = lp->dirty_tx % TX_RING_SIZE;
		unsigned xentry;

		if (!(lp->tx_ring[entry].control & DMAD_FINISHED)) {
			if (!stopped) break;
			// XXX: occasionally dma engine somehow skips one or two descriptors. this check works around this, so that we do not have to go tx timeout path
			xentry = (lp->dirty_tx + 40) % TX_RING_SIZE;
			if (!(lp->tx_ring[xentry].control & DMAD_FINISHED)) {
				break;
			}
//			printk("pass on\n");
		}

		++lp->dirty_tx;
		if (!lp->tx_skb[entry]) continue;

		devcs = lp->tx_ring[entry].devcs;

	    	if (devcs & DEVCS_TX_OK) {
			lp->dev->stats.tx_packets++;
			lp->dev->stats.tx_bytes += lp->tx_skb[entry]->len;
	    	} else {
			lp->dev->stats.tx_errors++;
			lp->dev->stats.tx_dropped++;
			
			if (devcs & DEVCS_TX_UNDERFLOW) {
				lp->dev->stats.tx_fifo_errors++;
			} else if (devcs & DEVCS_TX_OVERSIZED) {
				lp->dev->stats.tx_aborted_errors++;
			} else if (devcs & DEVCS_TX_EXCESSIVE_DEFERAL) {
				lp->dev->stats.tx_carrier_errors++;
			} else if (devcs & DEVCS_TX_EXCESSIVE_COLLISIONS) {
				lp->dev->stats.collisions++;
			} else if (devcs & DEVCS_TX_LATE_COLLISION) {
				lp->dev->stats.tx_window_errors++;
			}
    		}

		dev_kfree_skb(lp->tx_skb[entry]);
		lp->tx_skb[entry] = NULL;
	}

	if (lp->cur_tx - lp->dirty_tx < TX_QUEUE_LEN - (TX_QUEUE_LEN / 4)) {
		if (stopped) {
//			printk("wake queue\n");
			netif_wake_queue(dev);
		}
	}
}

static int korina_hw_open(struct net_device *dev) {
	struct korina_private *lp = netdev_priv(dev);
	int i;

	dma_abort(dev, DMA_TX_CHANNEL);
	dma_abort(dev, DMA_RX_CHANNEL);

	/* reset ethernet logic */ 
	KORINA_REG(KORINA_CONTROL) = 0;
	while (KORINA_REG(KORINA_CONTROL) & ETHINTFC_RESETTING) {
		dev->trans_start = jiffies;
	}
	KORINA_REG(KORINA_CONTROL) = ETHINTFC_ENABLE;
	KORINA_REG(KORINA_MAX_FRAME) = PKT_BUF_SZ;

	lp->cur_tx = 0;
	lp->dirty_tx = 0;
	
  	for (i = 0; i < RX_RING_SIZE; i++) {
		if (lp->rx_skb[i] == NULL) {
			struct sk_buff *skb = dev_alloc_skb(PKT_BUF_SZ + 2);
			if (!skb) break;

			skb->dev = dev;
			skb_reserve(skb, 2);
			lp->rx_skb[i] = skb;
			lp->rx_skb_dma[i] = dma_map_single(
				NULL, skb->data, PKT_BUF_SZ + 2,
				DMA_FROM_DEVICE);
		}
		lp->rx_ring[i].control = DMAD_INT_ON_DONE | PKT_BUF_SZ;
		lp->rx_ring[i].devcs = 0;
                lp->rx_ring[i].addr = lp->rx_skb_dma[i];
		lp->rx_ring[i].link = DMA_DESCR(lp->rd_ring_dma, i + 1);

  	}
	lp->rx_ring[i - 1].link = 0;
	lp->dirty_rx = i - RX_RING_SIZE;

	lp->cur_rx = 0;
	lp->dirty_rx = 0;

	DMA_REG(DMA_RX_CHANNEL, DMA_STATUS) = 0;

	korina_start_rx(lp, lp->rd_ring_dma);

	DMA_REG(DMA_TX_CHANNEL, DMA_STATUS_MASK) =
		~(DMASM_FINISHED | DMASM_ERROR);
	DMA_REG(DMA_RX_CHANNEL, DMA_STATUS_MASK) =
		~(DMASM_DONE | DMASM_HALT | DMASM_ERROR);

	/* accept only broadcasts and unicasts */
	KORINA_REG(KORINA_ADDR_RECOGN) = ETHARC_BROADCAST;

	for (i = 0; i < 4; ++i) {
		KORINA_REG(KORINA_ADDR_LO + i * 8) =
			swab32(get_unaligned((u32 *) &dev->dev_addr[2]));
		KORINA_REG(KORINA_ADDR_HI + i * 8) =
			swab16(get_unaligned((u16 *) &dev->dev_addr[0]));
	}

	KORINA_REG(KORINA_CONFIG2) = ETHMAC2_PAD_ENABLE | ETHMAC2_CRC_ENABLE
		| ETHMAC2_FULL_DUPLEX;
	/* back to back inter packet gap */ 
	KORINA_REG(KORINA_BTB_IPACK_GAP) = 21;
	/* non back to back inter packet gap */ 
	KORINA_REG(KORINA_NBTB_IPACK_GAP) = 18;
	KORINA_REG(KORINA_MII_CONFIG) = MIIMCFG_RESET;
	mb();
	udelay(1000);
	KORINA_REG(KORINA_MII_CONFIG) = 0;
	KORINA_REG(KORINA_CLOCK) = (mips_hpt_frequency / MII_CLOCK + 1) & ~1;
	/* don't transmit until fifo contains 48 bytes */
	KORINA_REG(KORINA_FIFO_TX_THRE) = 48;
	KORINA_REG(KORINA_CONFIG1) = ETHMAC1_RX_ENABLE;

	korina_multicast_list(dev);
	return 0;
}

static int korina_open(struct net_device *dev)
{
	korina_hw_open(dev);

	netif_start_queue(dev);

	enable_irq(KORINA_DMA_RX_IRQ);
	enable_irq(KORINA_DMA_TX_IRQ);
	enable_irq(KORINA_UNDERFLOW_IRQ);
	return 0;
}

static int korina_hw_close(struct net_device *dev) {
	struct korina_private *lp = netdev_priv(dev);
	int i;

	DMA_REG(DMA_TX_CHANNEL, DMA_STATUS_MASK) |=
		DMASM_FINISHED | DMASM_ERROR;
	DMA_REG(DMA_RX_CHANNEL, DMA_STATUS_MASK) |=
		DMASM_DONE | DMASM_HALT | DMASM_ERROR;

	for (i = 0; i < TX_RING_SIZE; ++i) {
		if (lp->tx_skb[i]) {
			dev_kfree_skb_any(lp->tx_skb[i]);
			lp->tx_skb[i] = NULL;
		}
	}

	return 0;
}

static int korina_close(struct net_device *dev)
{
	netif_stop_queue(dev);

	disable_irq(KORINA_DMA_RX_IRQ);
	disable_irq(KORINA_DMA_TX_IRQ);
	disable_irq(KORINA_UNDERFLOW_IRQ);

	korina_hw_close(dev);
	return 0;
}

static irqreturn_t korina_und_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	struct korina_private *lp = netdev_priv(dev);

	printk("korina: tx underflow, resetting\n");

	spin_lock(&lp->lock);

	KORINA_REG(KORINA_CONTROL) &= ~ETHINTFC_UNDERFLOW;
	korina_hw_close(dev);
	korina_hw_open(dev);

	spin_unlock(&lp->lock);

	return IRQ_HANDLED;
}


static void korina_tx_timeout(struct net_device *dev)
{
	struct korina_private *lp = netdev_priv(dev);
	unsigned long flags;

	printk("korina: tx timeout\n");
	korina_dump(dev);
	spin_lock_irqsave(&lp->lock, flags);

	korina_hw_close(dev);
	korina_hw_open(dev);
	netif_start_queue(dev);

	spin_unlock_irqrestore(&lp->lock, flags);
}

static int korina_change_mtu(struct net_device *dev, int new_mtu) {
    if (new_mtu < 68 || new_mtu > 1600) return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}

static const struct net_device_ops korina_netdev_ops = {
	.ndo_open		= korina_open,
	.ndo_stop		= korina_close,
	.ndo_start_xmit		= korina_xmit,
	.ndo_set_rx_mode	= &korina_multicast_list,
	.ndo_tx_timeout		= korina_tx_timeout,
	.ndo_change_mtu		= korina_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= korina_ioctl,
};

static int korina_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	struct korina_private *lp;
	int i;

	request_region(KORINA_BASE, 0x24c, "korina");
	
	dev = alloc_etherdev(sizeof(struct korina_private));

	dev->base_addr = KORINA_BASE;
	dev->irq = KORINA_DMA_RX_IRQ;
	memcpy(dev->dev_addr, mips_mac_address, 6);

	lp = netdev_priv(dev);
	lp->dev = dev;

	lp->tx_ring = dma_alloc_coherent(
		NULL, DMA_DESCR(0, TX_RING_SIZE + RX_RING_SIZE),
		&lp->td_ring_dma, GFP_KERNEL);
	if (!lp->tx_ring) {
		printk("korina: can't allocate descriptors\n");
		return -ENOMEM;
	}

	lp->rx_ring = (void *) lp->tx_ring + DMA_DESCR(0, TX_RING_SIZE);
	lp->rd_ring_dma = DMA_DESCR(lp->td_ring_dma, TX_RING_SIZE);

	spin_lock_init(&lp->lock);

	dev->netdev_ops = &korina_netdev_ops;
	dev->watchdog_timeo = HZ;
	netif_napi_add(dev, &lp->napi, korina_poll, 64);
	napi_enable(&lp->napi);

	dev->l2mtu = 1600;

	SET_ETHTOOL_OPS(dev, &korina_ethtool_ops);
	lp->mii_if.dev = dev;
	lp->mii_if.mdio_read = mdio_read;
	lp->mii_if.mdio_write = mdio_write;
	lp->mii_if.phy_id_mask = 0x1f;
	lp->mii_if.reg_num_mask = 0x1f;
	lp->mii_if.phy_id = 1;

	if (request_irq(KORINA_DMA_RX_IRQ, &korina_dma_interrupt,
			IRQF_SHARED | IRQF_DISABLED,
			"korina rx", dev)) {
		printk("korina: could not request IRQ %d\n", KORINA_DMA_RX_IRQ);
		goto err;
	}
	if (request_irq(KORINA_DMA_TX_IRQ, &korina_dma_interrupt,
			IRQF_SHARED | IRQF_DISABLED,
			"korina tx", dev)) {
		printk("korina: could not request IRQ %d\n", KORINA_DMA_TX_IRQ);
		goto free_rx_irq;
	}
	
	if (request_irq(KORINA_UNDERFLOW_IRQ, &korina_und_interrupt,
			IRQF_SHARED | IRQF_DISABLED,
			"korina underflow", dev)) {
		printk("korina: could not request IRQ %d\n",
		       KORINA_UNDERFLOW_IRQ);
		goto free_both_irq;
	}
	disable_irq(KORINA_DMA_RX_IRQ);
	disable_irq(KORINA_DMA_TX_IRQ);
	disable_irq(KORINA_UNDERFLOW_IRQ);

	platform_set_drvdata(pdev, dev);
	register_netdev(dev);

        printk("korina ethernet MAC address ");
	for (i = 0; i < 5; i++)
		printk("%2.2x:", dev->dev_addr[i]);
	printk("%2.2x\n", dev->dev_addr[5]);
    
	return 0;
free_both_irq:
	free_irq(KORINA_DMA_TX_IRQ, dev);
free_rx_irq:
	free_irq(KORINA_DMA_RX_IRQ, dev);
err:
	return -EAGAIN;
}

static struct platform_driver korina_driver = {
	.probe	= korina_probe,
	.driver	= {
		.name = "korina",
		.owner = THIS_MODULE,
	},
};

static int korina_init_module(void)
{
	return platform_driver_register(&korina_driver);
}

module_init(korina_init_module)
