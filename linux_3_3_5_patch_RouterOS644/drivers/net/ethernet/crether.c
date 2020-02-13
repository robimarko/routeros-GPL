#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/rb/cr.h>

#define TX_RING_SIZE	128
#define TX_QUEUE_LEN	120
#define RX_RING_SIZE	128

#define PACKET_BUF_LEN 1636


#define DMA_BASE (CR_ETHER_BASE + 0x1000)

#define CTR_REG(reg) \
        ((*(volatile u32 *) KSEG1ADDR(CR_CNTRL_BASE + (reg))))
#define MAC_REG(reg) \
        ((*(volatile u32 *) KSEG1ADDR(CR_ETHER_BASE + (reg))))
#define DMA_REG(reg) \
        ((*(volatile u32 *) KSEG1ADDR(DMA_BASE + (reg))))


#define MAC_CONTROL	    0x00
#define MAC_ADDR_HIGH       0x04
#define MAC_ADDR_LOW        0x08
#define MAC_MULTI_HASH_HIGH 0x0C
#define MAC_MULTI_HASH_LOW  0x10
#define MAC_MII_ADDR        0x14
#define MAC_MII_DATA        0x18
#define MAC_FLOW_CONTROL    0x1C

#define DMA_BUS_MODE       0x00
#define DMA_TX_POLL_DEMAND 0x04
#define DMA_RX_POLL_DEMAND 0x08
#define DMA_RX_BASE_ADDR   0x0C
#define DMA_TX_BASE_ADDR   0x10
#define DMA_STATUS         0x14
#define DMA_CONTROL        0x18
#define DMA_INT_ENABLE     0x1C
#define DMA_MISSED_COUNT   0x20
#define DMA_TX_CURR_ADDR   0x50
#define DMA_RX_CURR_ADDR   0x54

// MAC_CONTROL
#define MAC_FILTER_OFF         0x80000000 // Receive all incoming packets RW
#define MAC_BIG_ENDIAN         0x40000000 // Big endian mode RW
#define MAC_HEART_BEAT_OFF     0x10000000 // Heartbeat signal qual disable RW
#define MAC_SELECT_SRL         0x08000000 // Select SRL port RW
#define MAC_DISABLE_RX_OWN     0x00800000 // Disable receive own packets RW
#define MAC_LOOPBACK_EXT       0x00400000 // External loopback RW
#define MAC_LOOPBACK_INT       0x00200000 // Internal loopback
#define MAC_FULL_DUPLEX        0x00100000 // Full duplex mode RW
#define MAC_MCAST_FILTER_OFF   0x00080000 // Pass all multicast packets RW
#define MAC_PROMISC_MODE_ON    0x00040000 // Receive all valid packets RW 1
#define MAC_FILTER_INVERSE     0x00020000 // Inverse filtering RW
#define MAC_BAD_FRAMES_ENABLE  0x00010000 // Pass bad frames RW
#define MAC_PERFECT_FILTER_OFF 0x00008000 // Hash filtering only RW
#define MAC_HASH_FILTER_ON     0x00002000 // perform hash filtering RW
#define MAC_LATE_COLLISION_ON  0x00001000 // Enable late collision control RW
#define MAC_BROADCAST_DISABLE  0x00000800 // Disable reception of bcast RW
#define MAC_RETRY_DISABLE      0x00000400 // Disable retransmission RW
#define MAC_PAD_STRIP_ENABLE   0x00000100 // Pad stripping enable RW
#define MAC_DEFERRAL_ENABLE    0x00000020 // Deferral check enable RW
#define MAC_TX_ENABLE          0x00000008 // Transmitter enable RW
#define MAC_RX_ENABLE          0x00000004 // Receiver enable RW

// MAC_MII_ADDR
#define MII_DEV_MASK  0x0000F800 // MII device address
#define MII_DEV_SHIFT         11
#define MII_REG_MASK  0x000007C0 // MII register
#define MII_REG_SHIFT          6
#define MII_WRITE     0x00000002 // Write to register
#define MII_READ               0 // Read from register
#define MII_BUSY      0x00000001 // MII interface is busy

// MAC_MII_DATA
#define MII_DATA_MASK 0x0000FFFF // MII Data

// MAC_FLOW_CONTROL
#define MAC_PAUSE_TIME_MASK       0xFFFF0000  // PAUSE TIME field in ctrl frame
#define MAC_PAUSE_TIME_SHIFT              15
#define MAC_CONTROL_FRAME_ENABLE  0x00000004  // pass ctrl frames to host
#define MAC_CONTROL_FRAME_DISABLE          0  // Don't pass ctrl frames to host
#define MAC_FLOW_CONTROL_ENABLE   0x00000002  // Enable flow control
#define MAC_FLOW_CONTROL_DISABLE           0  // Disable flow control
#define MAC_SEND_PAUSE_FRAME      0x00000001  // send pause frame


// DMA_BUS_MODE
#define DMA_RX_ALIGN16           0x01000000 // rx to align on odd hw bndry
#define DMA_BIG_ENDIAN_DES       0x00100000 // Big endian data buffer descr RW
#define DMA_BURST_LENGTH32       0x00002000 // DMA_ burst length 32 RW
#define DMA_BURST_LENGTH_16      0x00001000 // DMA_ burst length 16
#define DMA_BURST_LENGTH_8       0x00000800 // DMA_ burst length 8
#define DMA_BURST_LENGHT_4       0x00000400 // DMA_ burst length 4
#define DMA_BURST_LENGTH_2       0x00000200 // DMA_ burst length 2
#define DMA_BURST_LENGTH_1       0x00000100 // DMA_ burst length 1
#define DMA_BURST_LENGTH_0       0x00000000 // DMA_ burst length 0
#define DMA_BIG_ENDIAN_DATA      0x00000080 // Big endian data buffers RW
#define DMA_DESCRIPTOR_SKIP_16   0x00000040 // number of dwords to skip RW
#define DMA_DESCRIPTOR_SKIP_8    0x00000020 // between two unchained descr
#define DMA_DESCRIPTOR_SKIP_4    0x00000010
#define DMA_DESCRIPTOR_SKIP_2    0x00000008
#define DMA_DESCRIPTOR_SKIP_1    0x00000004
#define DMA_DESCRIPTOR_SKIP_0             0
#define DMA_RECEIVE_PRIORITY_OFF 0x00000002 // equal rx and tx priorities RW
#define DMA_RECEIVE_PRIORITY_ON           0 // Rx has prioryty over Tx 0
#define DMA_RESET_ON             0x00000001 // Reset DMA engine RW

// DMA_STATUS
#define DMA_RX_ABORT         0x01000000 // receiver bus abort R 0
#define DMA_TX_ABORT         0x00800000 // transmitter bus abort R 0

// DMA_CONTROL
#define DMA_STORE_AND_FORWARD 0x00000000 // Store and forward RW 0
#define DMA_TX_THRESH_256     0x0000c000 // Non-SF threshold is 256 words
#define DMA_TX_THRESH_128     0x00008000 // Non-SF threshold is 128 words
#define DMA_TX_THRESH_64      0x00004000 // Non-SF threshold is 64 words
#define DMA_TX_THRESH_32      0x00000000 // Non-SF threshold is 32 words
#define DMA_TX_START          0x00002000 // Start/Stop transmission RW 0
#define DMA_TX_SECOND_FRAME   0x00000004 // Operate on second frame RW 0
#define DMA_RX_START          0x00000002 // Start/Stop reception RW 0

// DMA_INT_ENABLE
#define DMA_INT_NORMAL       0x00010000 // Normal interrupt summary RW 0
#define DMA_INT_ABNORMAL     0x00008000 // Abnormal interrupt summary RW 0
#define DMA_INT_EARLY_RX     0x00004000 // Early recv interrupt (Normal) RW 0
#define DMA_INT_BUS_ERROR    0x00002000 // Fatal bus error (Abnormal) RW 0
#define DMA_INT_EARLY_TX     0x00000400 // Early transmit interrupt RW 0
#define DMA_INT_RX_STOPPED   0x00000100 // Recv process stopped (Abnormal) RW 0
#define DMA_INT_RX_NO_BUFFER 0x00000080 // Recv buffer unavail (Abnormal) RW 0
#define DMA_INT_RX_COMPLETED 0x00000040 // Completion of rcption (Normal) RW 0
#define DMA_INT_TX_UNDERFLOW 0x00000020 // Trans underflow (Abnormal) RW 0
#define DMA_INT_TX_JABBER    0x00000008 // Trans Jabber Timeout (Abnormal) RW 0
#define DMA_INT_TX_NO_BUFFER 0x00000004 // Trans buffer unavailable (Normal) RW
#define DMA_INT_TX_STOPPED   0x00000002 // Trans process stopped (Abnormal) RW0
#define DMA_INT_TX_COMPLETED 0x00000001 // Trans completed (Normal) RW 0


#define DMA_INT_ALL \
      ( DMA_INT_NORMAL  \
      | DMA_INT_ABNORMAL  \
      | DMA_INT_EARLY_RX  \
      | DMA_INT_BUS_ERROR \
      | DMA_INT_EARLY_TX  \
      | DMA_INT_RX_STOPPED \
      | DMA_INT_RX_NO_BUFFER \
      | DMA_INT_RX_COMPLETED \
      | DMA_INT_TX_UNDERFLOW \
      | DMA_INT_TX_JABBER \
      | DMA_INT_TX_NO_BUFFER \
      | DMA_INT_TX_STOPPED \
      | DMA_INT_TX_COMPLETED)

#define DMA_INT_INTERESTING \
      ( DMA_INT_NORMAL  \
      | DMA_INT_ABNORMAL  \
      | DMA_INT_BUS_ERROR \
      | DMA_INT_RX_COMPLETED \
      | DMA_INT_TX_COMPLETED)

#define DMA_INT_TX \
      ( DMA_INT_TX_COMPLETED)

#define DMA_INT_RX \
      ( DMA_INT_RX_COMPLETED)



// DMA Engine descriptor layout
// status word of DMA descriptor
#define DESC_OWN_BY_DMA       0x80000000 // Descriptor is owned by DMA engine
#define DESC_FRAME_LEN_MASK   0x3FFF0000 // Receive descriptor frame length
#define DESC_FRAME_LEN_SHIFT          16
#define DESC_ERROR            0x00008000 // Error summary bit OR of following
#define DESC_RX_TRUNCATED     0x00004000 // Rx - no more descs for recv frame
#define DESC_RX_LENGTH_ERROR  0x00001000 // Rx - frame size != length field
#define DESC_RX_RUNT          0x00000800 // Rx - runt frame
#define DESC_RX_MULTICAST     0x00000400 // Rx - received frame is multicast
#define DESC_RX_FIRST         0x00000200 // Rx - first descriptor of the frame
#define DESC_RX_LAST          0x00000100 // Rx - last descriptor of the frame
#define DESC_RX_LONG_FRAME    0x00000080 // Rx - frame is longer than 1518 b
#define DESC_RX_LATE_COLL     0x00000040 // Rx - frame damaged by late collsion
#define DESC_RX_FRAME_ETHER   0x00000020 // Rx - Frame type Ethernet 802.3
#define DESC_RX_MII_ERROR     0x00000008 // Rx - error reported by MII iface
#define DESC_RX_DRIBLING      0x00000004 // Rx - frame no multiple of 8 bits
#define DESC_RX_CRC           0x00000002 // Rx - CRC error
#define DESC_TX_TIMEOUT       0x00004000 // Tx - Transmit jabber timeout
#define DESC_TX_LOST_CARRIER  0x00000800 // Tx - carrier lost during trans
#define DESC_TX_NO_CARRIER    0x00000400 // Tx - no carrier from tranceiver
#define DESC_TX_LATE_COLL     0x00000200 // Tx - trans aborted due to collision
#define DESC_TX_EXC_COLLS     0x00000100 // Tx - trans aborted after 16 colls
#define DESC_TX_HEARBEAT_FAIL 0x00000080 // Tx - heartbeat collision check fail
#define DESC_TX_COLL_MASK     0x00000078 // Tx - Collision count
#define DESC_TX_COLL_SHIFT             3
#define DESC_TX_EXC_DEFERRAL  0x00000004 // Tx - excessive deferral
#define DESC_TX_UNDERFLOW     0x00000002 // Tx - late data arrival from memory
#define DESC_TX_DEFERRED      0x00000001 // Tx - frame transmision deferred

// control word of DMA descriptor
#define DESC_TX_INT_ENABLE    0x80000000 // Tx - interrupt on completion
#define DESC_TX_LAST          0x40000000 // Tx - Last segment of the frame
#define DESC_TX_FIRST         0x20000000 // Tx - First segment of the frame
#define DESC_TX_DISABLE_CRC   0x04000000 // Tx - Add CRC disabled
#define DESC_END_OF_RING      0x02000000 // End of descriptors ring
#define DESC_CHAIN            0x01000000 // Second buffer address is chain
#define DESC_TX_DISABLE_PAD   0x00800000 // disable padding
#define DESC_SIZE_2_MASK      0x003FF800 // Buffer 2 size
#define DESC_SIZE_2_SHIFT             11
#define DESC_SIZE_1_MASK      0x000007FF // Buffer 1 size
#define DESC_SIZE_1_SHIFT              0

#define DMA_DESCR(base, off) ((base) + sizeof(struct dma_desc) * (off))


extern unsigned char mips_mac_address[6];

struct dma_desc {
    u32 status;
    u32 control;
    u32 addr;
    u32 next;
};

/*
static void dump_regs(void) {
    unsigned status = DMA_REG(DMA_STATUS);
    printk("MAC_CONTROL      %08x  FLOWC %08x\n", MAC_REG(MAC_CONTROL), MAC_REG(MAC_FLOW_CONTROL));
//    printk("MAC_ADDR_*       %08x/%08x  MH %08x/%08x\n",
//	   MAC_REG(MAC_ADDR_HIGH), MAC_REG(MAC_ADDR_LOW),
//	   MAC_REG(MAC_MULTI_HASH_HIGH), MAC_REG(MAC_MULTI_HASH_LOW));
//    printk("MAC_MII_ADDR        %08x\n", MAC_REG(MAC_MII_ADDR));
//    printk("MAC_MII_DATA        %08x\n", MAC_REG(MAC_MII_DATA));

    printk("DMA  B/S %08x/%08x   C/I %08x/%08x   %x %x %x\n",
	   DMA_REG(DMA_BUS_MODE), status, DMA_REG(DMA_CONTROL),
	   DMA_REG(DMA_INT_ENABLE),
	   (status >> 17) & 7, (status >> 20) & 7, (status >> 23) & 7);
//    printk("DMA_MISSED_COUNT  %08x\n", DMA_REG(DMA_MISSED_COUNT));
//    printk("DMA_*_POLL %08x/%08x\n",
//	   DMA_REG(DMA_TX_POLL_DEMAND), DMA_REG(DMA_RX_POLL_DEMAND));
    printk("DMA BASE %08x/%08x  CURR %08x/%08x\n",
	   DMA_REG(DMA_TX_BASE_ADDR), DMA_REG(DMA_RX_BASE_ADDR),
	   DMA_REG(DMA_TX_CURR_ADDR), DMA_REG(DMA_RX_CURR_ADDR));
}

static void dump_descr_regs(struct dma_desc *d) {
    printk("DESCR: %08x %08x %08x %08x\n",
	   d->status, d->control, d->addr, d->next);
}
*/

struct crether_private {
    volatile struct dma_desc *tx_ring;
    volatile struct dma_desc *rx_ring;
    dma_addr_t td_ring_dma;
    dma_addr_t rd_ring_dma;

    struct sk_buff *tx_skb[TX_RING_SIZE];
    struct sk_buff *rx_skb[RX_RING_SIZE];
    dma_addr_t rx_skb_dma[RX_RING_SIZE];

    struct tasklet_struct *rx_tasklet;
    struct tasklet_struct *tx_tasklet;

    unsigned cur_rx;
    unsigned dirty_rx;

    unsigned cur_tx;
    unsigned dirty_tx;

    struct net_device *dev;
    spinlock_t lock;

    struct mii_if_info mii_if;
};


static int mdio_read(struct net_device *dev, int phy_id, int regnum)
{
    while (MAC_REG(MAC_MII_ADDR) & MII_BUSY) {}

    MAC_REG(MAC_MII_ADDR) =
	(phy_id << MII_DEV_SHIFT) | (regnum << MII_REG_SHIFT);

    while (MAC_REG(MAC_MII_ADDR) & MII_BUSY) {}

    return (MAC_REG(MAC_MII_DATA) & MII_DATA_MASK);
}

static void mdio_write(struct net_device *dev,
		       int phy_id, int regnum, int value)
{
    while (MAC_REG(MAC_MII_ADDR) & MII_BUSY) {}

    MAC_REG(MAC_MII_DATA) = value;
    MAC_REG(MAC_MII_ADDR) =
	(phy_id << MII_DEV_SHIFT) | (regnum << MII_REG_SHIFT) | MII_WRITE;

    while (MAC_REG(MAC_MII_ADDR) & MII_BUSY) {}
}


static void crether_get_drvinfo (struct net_device *dev,
				 struct ethtool_drvinfo *info)
{
    strcpy(info->driver, "crether");
    strcpy(info->version, "1.0");
    strcpy(info->bus_info, "00:00.0 crether");
}

static int crether_get_settings(struct net_device *dev,
				struct ethtool_cmd *cmd)
{
    struct crether_private *np = netdev_priv(dev);

    return mii_ethtool_gset(&np->mii_if, cmd);
}

static int crether_set_settings(struct net_device *dev,
				struct ethtool_cmd *cmd)
{
    struct crether_private *np = netdev_priv(dev);
    int rc;

    spin_lock_irq(&np->lock);
    rc = mii_ethtool_sset(&np->mii_if, cmd);
    spin_unlock_irq(&np->lock);

    return rc;
}

static int crether_nway_reset(struct net_device *dev)
{
    struct crether_private *np = netdev_priv(dev);

    return mii_nway_restart(&np->mii_if);
}

static u32 crether_get_link(struct net_device *dev)
{
    struct crether_private *np = netdev_priv(dev);

    return mii_link_ok(&np->mii_if);
}

static void crether_multicast_list(struct net_device *dev)
{   
    if ((dev->flags & IFF_PROMISC) ||
	(dev->flags & IFF_ALLMULTI) ||
	(netdev_mc_count(dev) > 0)) {
	MAC_REG(MAC_CONTROL) |= MAC_PROMISC_MODE_ON;
    }
    else {
	MAC_REG(MAC_CONTROL) &= ~MAC_PROMISC_MODE_ON;
    }
}


static struct ethtool_ops crether_ethtool_ops = {
    .get_drvinfo	= crether_get_drvinfo,
    .get_settings	= crether_get_settings,
    .set_settings	= crether_set_settings,
    .nway_reset		= crether_nway_reset,
    .get_link		= crether_get_link,
};

static int crether_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct crether_private *lp = netdev_priv(dev);
    int rc;

//    if (!netif_running(dev))
//	return -EINVAL;

    spin_lock_irq(&lp->lock);
    rc = generic_mii_ioctl(
	&lp->mii_if, (struct mii_ioctl_data *) &rq->ifr_data, cmd, NULL);
    spin_unlock_irq(&lp->lock);

    return rc;
}

static int crether_init(struct net_device *dev)
{
    struct crether_private *lp = netdev_priv(dev);
    int i;

    //printk("***** crether_init\n");
    // total reset of ethernet
    CTR_REG(4) |= 3 << 10;
    udelay(30);
    CTR_REG(4) &= ~(3 << 10);
//    dump_regs();

    tasklet_disable(lp->rx_tasklet);
    tasklet_disable(lp->tx_tasklet);

    lp->cur_tx = 0;
    lp->dirty_tx = 0;

    for (i = 0; i < RX_RING_SIZE; i++) {
	if (lp->rx_skb[i] == NULL) {
	    struct sk_buff *skb = netdev_alloc_skb(dev, PACKET_BUF_LEN + 2);
	    if (!skb) break;

	    skb->dev = dev;
	    skb_reserve(skb, 2);
	    lp->rx_skb[i] = skb;
	    lp->rx_skb_dma[i] = dma_map_single(
		NULL, skb->data, PACKET_BUF_LEN, DMA_FROM_DEVICE);
	}
	lp->rx_ring[i].status = DESC_OWN_BY_DMA;
	lp->rx_ring[i].control =
	    DESC_CHAIN | (PACKET_BUF_LEN << DESC_SIZE_1_SHIFT);
	lp->rx_ring[i].addr = lp->rx_skb_dma[i];
	lp->rx_ring[i].next = DMA_DESCR(lp->rd_ring_dma, i + 1);
    }
    lp->rx_ring[i - 1].next = 0;
    lp->rx_ring[i - 1].control = DESC_END_OF_RING;
    lp->cur_rx = 0;
    lp->dirty_rx = 0;

    for (i = 0; i < TX_RING_SIZE; i++) {
	lp->tx_ring[i].status = 0;
	lp->tx_ring[i].control = DESC_CHAIN;
	lp->tx_ring[i].addr = 0;
	lp->tx_ring[i].next = DMA_DESCR(lp->td_ring_dma, i + 1);
    }
    lp->tx_ring[i - 1].next = 0;
    lp->tx_ring[i - 1].control |= DESC_END_OF_RING;


    MAC_REG(MAC_CONTROL) = MAC_FULL_DUPLEX;
    DMA_REG(DMA_BUS_MODE) = DMA_BURST_LENGTH_2;

    DMA_REG(DMA_STATUS) = DMA_INT_ALL;
    DMA_REG(DMA_RX_BASE_ADDR) = lp->rd_ring_dma;
    DMA_REG(DMA_TX_BASE_ADDR) = lp->td_ring_dma;
    DMA_REG(DMA_CONTROL) = DMA_TX_THRESH_64;
    DMA_REG(DMA_CONTROL) |= DMA_RX_START | DMA_TX_START;
    MAC_REG(MAC_CONTROL) |= MAC_RX_ENABLE | MAC_TX_ENABLE;

    DMA_REG(DMA_INT_ENABLE) = DMA_INT_INTERESTING;

    tasklet_enable(lp->rx_tasklet);
    tasklet_enable(lp->tx_tasklet);

    netif_start_queue(dev);

//    dump_regs();
    //printk("***** crether_init out\n");

    return 0; 
}

static int crether_restart(struct net_device *dev)
{
    disable_irq(CR_ETH_IRQ);

    crether_init(dev);
    crether_multicast_list(dev);

    enable_irq(CR_ETH_IRQ);
    return 0;
}


static int crether_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct crether_private *lp = netdev_priv(dev);
    unsigned long flags;
    unsigned int entry;
    volatile struct dma_desc *td;

    if (skb_padto(skb, ETH_ZLEN))
	return 0;

    spin_lock_irqsave(&lp->lock, flags);

//    printk("***** crether_xmit\n");
    entry = lp->cur_tx % TX_RING_SIZE;
    td = &lp->tx_ring[entry];

    lp->tx_skb[entry] = skb;
    td->status = 0;
    td->control = DESC_CHAIN | DESC_TX_FIRST | DESC_TX_LAST |
	DESC_TX_INT_ENABLE | (skb->len << DESC_SIZE_1_SHIFT);

    if (entry == TX_RING_SIZE - 1) td->control |= DESC_END_OF_RING;

    td->addr = dma_map_single(NULL, skb->data, skb->len, DMA_TO_DEVICE);
    td->status = DESC_OWN_BY_DMA;

    DMA_REG(DMA_TX_POLL_DEMAND) = 0;

    ++lp->cur_tx;

    if (lp->cur_tx == lp->dirty_tx + TX_QUEUE_LEN)
	netif_stop_queue(dev);

    dev->trans_start = jiffies;

    spin_unlock_irqrestore(&lp->lock, flags);

    return 0;
}

static irqreturn_t crether_interrupt(int irq, void *dev_id)
{
    struct net_device *dev = (struct net_device *) dev_id;
    struct crether_private *lp = netdev_priv(dev);
    unsigned status;
    spin_lock(&lp->lock);

    status = DMA_REG(DMA_STATUS);
    DMA_REG(DMA_STATUS) = status & DMA_INT_ALL;

//    printk("***** crether irq: %04x\n", status);
//    dump_regs();
    if (status & (DMA_INT_BUS_ERROR)) {
	printk("crether:%s\n",
	       (status & DMA_INT_BUS_ERROR) ? " BUS_ERROR" : "");
	crether_restart(dev);
	spin_unlock(&lp->lock);
	return IRQ_HANDLED;
    }

    if (status & DMA_INT_RX_COMPLETED) {
	tasklet_hi_schedule(lp->rx_tasklet);
	DMA_REG(DMA_INT_ENABLE) &= ~(DMA_INT_RX);
    }
    if (status & DMA_INT_TX_COMPLETED) {
	tasklet_hi_schedule(lp->tx_tasklet);
	DMA_REG(DMA_INT_ENABLE) &= ~(DMA_INT_TX);
    }

    spin_unlock(&lp->lock);
    return IRQ_HANDLED;
}


static void crether_rx_tasklet(unsigned long rx_data_dev)
{
    struct net_device *dev = (struct net_device *)rx_data_dev;	
    struct crether_private* lp = netdev_priv(dev);
    unsigned long flags;

    int boguscnt = lp->dirty_rx + RX_RING_SIZE - lp->cur_rx;

    spin_lock_irqsave(&lp->lock, flags);

    // keep going while we have received into more descriptors
    while (1) {
	unsigned entry = lp->cur_rx % RX_RING_SIZE;
	volatile struct dma_desc *rd = &lp->rx_ring[entry];
	unsigned pktlen;
	unsigned status;

	if (rd->status & DESC_OWN_BY_DMA) break;
	if (--boguscnt < 0) break;

	status = rd->status;
	if (status & DESC_ERROR && !(status & DESC_RX_LONG_FRAME)) {
	    lp->dev->stats.rx_errors++;
	    lp->dev->stats.rx_dropped++;

	    if (status & DESC_RX_CRC) {
		lp->dev->stats.rx_crc_errors++;
	    }
	    else if (status & DESC_RX_LENGTH_ERROR) {
		lp->dev->stats.rx_length_errors++;
	    }
//	    else if (status & DESC_RX_LONG_FRAME) {
//		lp->dev->stats.rx_length_errors++;
//	    }
	}
	else {
	    struct sk_buff *skb;

	    pktlen = ((status >> 16) & 0x3FFF) - 4;

	    skb = lp->rx_skb[entry];
	    lp->rx_skb[entry] = NULL;
	    dma_unmap_single(NULL, lp->rx_skb_dma[entry], pktlen,
			     DMA_FROM_DEVICE);

	    skb_put(skb, pktlen);
	    skb->protocol = eth_type_trans(skb, dev);
	    netif_rx(skb);

	    dev->last_rx = jiffies;
	    lp->dev->stats.rx_packets++;
	    lp->dev->stats.rx_bytes += pktlen;
	}
	++lp->cur_rx;
    }

    // refill the rx ring buffers
    for (; lp->cur_rx - lp->dirty_rx > 0; ++lp->dirty_rx) {
	unsigned entry = lp->dirty_rx % RX_RING_SIZE;
	volatile struct dma_desc *rd = &lp->rx_ring[entry];
	struct sk_buff *skb;

	if (lp->rx_skb[entry] == NULL) {
	    skb = netdev_alloc_skb(dev, PACKET_BUF_LEN + 2);
	    lp->rx_skb[entry] = skb;
	    if (skb == NULL) break;

	    skb->dev = dev;
	    skb_reserve(skb, 2);
	    lp->rx_skb_dma[entry] = dma_map_single(
		NULL, skb->data, PACKET_BUF_LEN + 2,
		DMA_FROM_DEVICE);
	}
	rd->addr = lp->rx_skb_dma[entry];
	rd->status = DESC_OWN_BY_DMA;
    }

    DMA_REG(DMA_STATUS) &= ~(DMA_INT_RX_COMPLETED);
    DMA_REG(DMA_INT_ENABLE) |= DMA_INT_RX;
    DMA_REG(DMA_RX_POLL_DEMAND) = 0;
    spin_unlock_irqrestore(&lp->lock, flags);
}

static void crether_tx_tasklet(unsigned long tx_data_dev)
{
    struct net_device *dev = (struct net_device *) tx_data_dev;
    struct crether_private* lp = netdev_priv(dev);
    u32 status;
    unsigned long flags;

    spin_lock_irqsave(&lp->lock, flags);

    while (lp->dirty_tx != lp->cur_tx) {
	unsigned entry = lp->dirty_tx % TX_RING_SIZE;

	status = lp->tx_ring[entry].status;
	if (status & DESC_OWN_BY_DMA) break;

	if (!(status & DESC_ERROR)) {
	    lp->dev->stats.tx_packets++;
	    lp->dev->stats.tx_bytes += lp->tx_skb[entry]->len;
	} else {
	    lp->dev->stats.tx_errors++;
	    lp->dev->stats.tx_dropped++;

	    if (status & DESC_TX_UNDERFLOW) {
		lp->dev->stats.tx_fifo_errors++;
	    } else if (status & DESC_TX_EXC_DEFERRAL) {
		lp->dev->stats.tx_carrier_errors++;
	    } else if (status & DESC_TX_LOST_CARRIER) {
		lp->dev->stats.tx_carrier_errors++;
	    } else if (status & DESC_TX_NO_CARRIER) {
		lp->dev->stats.tx_carrier_errors++;
	    } else if (status & DESC_TX_EXC_COLLS) {
		lp->dev->stats.collisions++;
	    } else if (status & DESC_TX_LATE_COLL) {
		lp->dev->stats.tx_window_errors++;
	    } else if (status & DESC_TX_HEARBEAT_FAIL) {
		lp->dev->stats.tx_heartbeat_errors++;
	    }
	}

	dev_kfree_skb_any(lp->tx_skb[entry]);
	lp->tx_skb[entry] = NULL;
	++lp->dirty_tx;
    }

    if (lp->cur_tx - lp->dirty_tx < TX_QUEUE_LEN - 4)
	netif_wake_queue(dev);

    DMA_REG(DMA_STATUS) &= ~(DMA_INT_TX_COMPLETED);
    DMA_REG(DMA_INT_ENABLE) |= DMA_INT_TX;
    spin_unlock_irqrestore(&lp->lock, flags);
}

static void crether_tx_timeout(struct net_device *dev)
{
    struct crether_private *lp = netdev_priv(dev);
    unsigned long flags;

    spin_lock_irqsave(&lp->lock, flags);
    crether_restart(dev);
    spin_unlock_irqrestore(&lp->lock, flags);
}

static int crether_change_mtu(struct net_device *dev, int new_mtu) {
    if (new_mtu < 68 || new_mtu > 1600) return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}

static int crether_open(struct net_device *dev)
{
    if (crether_init(dev)) {
	printk("crether: could not init device\n");
	return -EAGAIN;
    }

    if (request_irq(CR_ETH_IRQ, &crether_interrupt,
		    IRQF_SHARED | IRQF_DISABLED, "crether", dev)) {
	printk("crether: could not request IRQ %d\n", CR_ETH_IRQ);
	return -EAGAIN;
    }
    return 0;
}

static int crether_close(struct net_device *dev)
{
    disable_irq(CR_ETH_IRQ);

    MAC_REG(MAC_CONTROL) &= ~(MAC_RX_ENABLE | MAC_TX_ENABLE);
    DMA_REG(DMA_CONTROL) &= ~(DMA_RX_START | DMA_TX_START);

    free_irq(CR_ETH_IRQ, dev);

    return 0;
}

static const struct net_device_ops crether_netdev_ops = {
	.ndo_open		= crether_open,
	.ndo_stop		= crether_close,
	.ndo_start_xmit		= crether_xmit,
	.ndo_set_rx_mode	= &crether_multicast_list,
	.ndo_tx_timeout		= crether_tx_timeout,
	.ndo_change_mtu		= crether_change_mtu,
	.ndo_do_ioctl		= crether_ioctl,
	.ndo_validate_addr	= eth_validate_addr,
};

static int crether_probe(struct platform_device *pdev)
{
    struct net_device *dev;
    struct crether_private *lp;
    int i;

    request_region(CR_ETHER_BASE, 0x2000, "crether");

    dev = alloc_etherdev(sizeof(struct crether_private));

    dev->base_addr = CR_ETHER_BASE;
    dev->irq = CR_ETH_IRQ;
    memcpy(dev->dev_addr, mips_mac_address, 6);

    lp = netdev_priv(dev);
    lp->dev = dev;

    lp->tx_ring = dma_alloc_coherent(
	&pdev->dev, DMA_DESCR(0, TX_RING_SIZE + RX_RING_SIZE),
	&lp->td_ring_dma, GFP_KERNEL);
    if (!lp->tx_ring) {
	printk("crether: can't allocate descriptors\n");
	return -ENOMEM;
    }

    lp->rx_ring = (void *) lp->tx_ring + DMA_DESCR(0, TX_RING_SIZE);
    lp->rd_ring_dma = DMA_DESCR(lp->td_ring_dma, TX_RING_SIZE);

    spin_lock_init(&lp->lock);

    dev->netdev_ops = &crether_netdev_ops;
    dev->ethtool_ops = &crether_ethtool_ops;
    dev->watchdog_timeo = 6 * HZ;
    dev->l2mtu = 1600;

    lp->rx_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    tasklet_init(lp->rx_tasklet, crether_rx_tasklet, (unsigned long)dev);
    lp->tx_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    tasklet_init(lp->tx_tasklet, crether_tx_tasklet, (unsigned long)dev);

    lp->mii_if.dev = dev;
    lp->mii_if.mdio_read = mdio_read;
    lp->mii_if.mdio_write = mdio_write;
    lp->mii_if.phy_id_mask = 0x1f;
    lp->mii_if.reg_num_mask = 0x1f;
    lp->mii_if.phy_id = 1;

    register_netdev(dev);

    printk("crether ethernet MAC address ");
    for (i = 0; i < 5; i++)
	printk("%2.2x:", dev->dev_addr[i]);
    printk("%2.2x\n", dev->dev_addr[5]);

    return 0;
}

static struct platform_driver crether_driver = {
	.probe	= crether_probe,
	.driver	= {
		.name	= "cr-ether",
		.owner	= THIS_MODULE,
	}
};

int crether_init_module(void)
{
	return platform_driver_register(&crether_driver);
}

module_init(crether_init_module);
