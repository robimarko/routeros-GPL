#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/etherdevice.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/if_arp.h>
#include <linux/platform_device.h>
#include <linux/if_switch.h>

#define ADMTEK_REG_BASE		0x12000000

// register offsets
#define Boot_done_REG	0x08
#define SWReset_REG	0x0C
#define PHY_st_REG	0x14
#define Port_st_REG	0x18
#define SW_conf_REG	0x20
#define CPUp_conf_REG	0x24
#define Port_conf0_REG	0x28
#define Port_conf1_REG	0x2C
#define Port_conf2_REG	0x30
#define VLAN_G1_REG	0x40
#define VLAN_G2_REG	0x44
#define Send_trig_REG	0x48
#define Srch_cmd_REG	0x4C
#define ADDR_st0_REG	0x50
#define ADDR_st1_REG	0x54
#define MAC_wt0_REG	0x58
#define MAC_wt1_REG	0x5C
#define BW_cntl0_REG	0x60
#define BW_cntl1_REG	0x64
#define PHY_cntl0_REG	0x68
#define PHY_cntl1_REG	0x6C
#define PHY_cntl2_REG	0x7C
#define PHY_cntl3_REG	0x80
#define Empty_cnt	0xA4
#define SW_Int_st_REG	0xB0
#define SW_Int_mask_REG	0xB4
#define GPIO_CONF0_REG	0xB8
#define GPIO_CONF2_REG	0xBC
#define Send_HBaddr_REG	0xD0
#define Send_LBaddr_REG	0xD4
#define Recv_HBaddr_REG	0xD8
#define Recv_LBaddr_REG	0xDC
#define Send_HWaddr_REG	0xE0
#define Send_LWaddr_REG	0xE4
#define Recv_HWaddr_REG	0xE8
#define Recv_LWaddr_REG	0xEC

#define ADMTEK_REG(reg) \
    (*((volatile unsigned long *)(KSEG1ADDR(ADMTEK_REG_BASE + (reg)))))


// CPUp_conf_REG
#define SW_CPU_PORT_DISABLE	0x00000001
#define SW_PADING_CRC		0x00000002
#define SW_BRIDGE_MODE		0x00000004

// Port_conf0_REG
#define SW_DISABLE_PORT_MASK	0x0000003F

// MAC_wt0_REG
#define SW_MAC_WRITE		0x00000001
#define SW_MAC_WRITE_DONE	0x00000002
#define SW_MAC_VLANID_EN	0x00000040
#define SW_MAC_VLANID_SHIFT	3
#define SW_MAC_PORT_SHIFT	3

// SW_Int_st_REG & SW_Int_mask_REG
#define TX_H_INT		0x00000001
#define TX_L_INT		0x00000002
#define RX_H_INT		0x00000004
#define RX_L_INT		0x00000008
#define RX_H_DESC_FULL_INT	0x00000010
#define RX_L_DESC_FULL_INT	0x00000020
#define CPU_QUEUE_FULL_INT	0x00002000
#define GLOBAL_QUEUE_FULL_INT	0x00004000
#define MUST_DROP_INT		0x00008000
#define PORT_STATUS_CHANGE_INT	0x00040000
#define RX_DESC_ERR_INT		0x00400000
#define TX_DESC_ERR_INT	        0x00800000
#define SWITCH_INT_MASK		0x01C4EFFF

#define INT_MASK (TX_L_INT | RX_L_INT | RX_L_DESC_FULL_INT)

// descriptor bits
#define DESC_OWN           0x80000000
#define DESC_RING_END      0x10000000
#define DESC_ADDR_MASK     0x00ffffff
#define DESC_LEN_MASK      0x07ff0000
#define DESC_LEN_SHIFT     16

// rx descr status
#define DESC_RX_PORT_MASK  0x00007000
#define DESC_RX_PORT_SHIFT 12
#define DESC_RX_CSUM_ERR   0x00000008
#define DESC_RX_VLAN_TAG   0x00000004


#define RX_BUF_SIZE 1550
#define SW_IRQ 17
#define EMPTY_THRESH 0x100
#define RX_DESC_COUNT 128
#define ADMTEK_PORT_COUNT 5
#define ICPLUS_PORT_COUNT 5
#define PORT_COUNT (ADMTEK_PORT_COUNT + ICPLUS_PORT_COUNT)
#define TX_ENA_THRESHOLD 24

#define ADMTEK_PORT(x) (x->num < 100)
#define ICPLUS_PORT(x) (x->num >= 100)

#define UP_PORT 5

static unsigned icplus_read_reg(unsigned phy, unsigned reg) {
    unsigned ret;
    ADMTEK_REG(PHY_cntl0_REG) = phy | reg << 8 | BIT(14);
    while (1) {
	ret = ADMTEK_REG(PHY_cntl1_REG);
	if (ret & BIT(1)) break;
    }

    ret >>= 16;
//    printk("icplus read %x %x %04x\n", phy, reg, ret);
    return ret;
}

static void icplus_write_reg(unsigned phy, unsigned reg, unsigned value) {
    unsigned ret;
//    printk("icplus write %x %x %04x\n", phy, reg, value);
    ADMTEK_REG(PHY_cntl0_REG) = phy | reg << 8 | BIT(13) | value << 16;
    while (1) {
	ret = ADMTEK_REG(PHY_cntl1_REG);
	if (ret & BIT(0)) break;
    }
}

/*
static void icplus_dump_regs(void) {
    unsigned i, j;
    unsigned v;

    printk("icplus regs:\n");

//    for (i = 0; i < 6; ++i) {
//	for (j = 0; j < 6; ++j) {
//	    printk("reg %d %d: %04x\n", i, j, icplus_read_reg(i, j));
//	}
//    }

    i = 29;
    for (j = 18; j < 32; ++j) {
	v = icplus_read_reg(i, j);
	printk("reg %d %d: %04x\n", i, j, v);
    }
    i = 30;
    for (j = 0; j < 32; ++j) {
	v = icplus_read_reg(i, j);
	printk("reg %d %d: %04x\n", i, j, v);
    }
    i = 31;
    for (j = 0; j < 7; ++j) {
	v = icplus_read_reg(i, j);
	printk("reg %d %d: %04x\n", i, j, v);
    }
}
*/

static void icplus_stop_engine(void) {
}

static void icplus_init_engine(void) {
    icplus_write_reg(29, 31, 0x175C);
    icplus_write_reg(30, 10, 0);
    icplus_write_reg(30, 0, 0x175C);
    mdelay(2);

//    icplus_write_reg(29, 18, 0xcc0c);
    icplus_write_reg(29, 19, BIT(15) | BIT(7));
    icplus_write_reg(29, 20, BIT(15) | BIT(7));
    icplus_write_reg(29, 21, BIT(15));
    icplus_write_reg(29, 23, (0x1F << 6) | BIT(1));
    icplus_write_reg(29, 24, 0xffff);
    icplus_write_reg(29, 25, 0xffff);
    icplus_write_reg(29, 26, 0xffff);
    icplus_write_reg(29, 27, 0xffff);
    icplus_write_reg(29, 28, 0xffff);
    icplus_write_reg(29, 30, 0xffff);

//    icplus_write_reg(30, 1, 0x3f3f);
//    icplus_write_reg(30, 2, 0x3f3f);
//    icplus_write_reg(30, 3, 0x3f3f);
//    icplus_write_reg(30, 4, 0x3f3f);
//    icplus_write_reg(30, 5, 0x3f3f);
//    icplus_write_reg(30, 6, 0x3f3f);
//    icplus_write_reg(30, 7, 0x3f3f);
//    icplus_write_reg(30, 8, 0x1f00);

//    icplus_write_reg(30, 9, BIT(7) | BIT(3) | 5);
    icplus_write_reg(30, 9, 5);
    icplus_write_reg(30, 12, BIT(7) | BIT(5) | BIT(1));
    icplus_write_reg(30, 13, 0xf);
    icplus_write_reg(30, 16, BIT(13) | BIT(7) | 0x1f);
    icplus_write_reg(30, 18, BIT(11) | BIT(8));
    icplus_write_reg(30, 26, 0);
    icplus_write_reg(31, 0, 0);
    icplus_write_reg(31, 1, 0);
    icplus_write_reg(31, 2, 0);
    icplus_write_reg(31, 6, BIT(14) | BIT(1) | BIT(0));

//    icplus_dump_regs();
}


struct admtek_rx_desc {
    unsigned long buf1;
    unsigned long buf2;
    unsigned long len;
    unsigned long status;
};

struct admtek_tx_desc {
    unsigned long buf1;
    unsigned long buf2;
    unsigned long len;
    unsigned long ctrl;
};

struct admtek_port {
    struct net_device *dev;
    struct mii_if_info mii_info;

    int num;
    struct admtek_port *master;
    int master_count;
    int master_ports;
    int master_links;

    int in_transmit;
    int link;

    struct timer_list pause_timer;
};

struct tx_head {
    unsigned char macs[12];
    unsigned char vlan0;
    unsigned char vlan1;
    unsigned char vlan2;
    unsigned char vlan3;
} __attribute__((packed));

struct admtek_switch {
    struct admtek_rx_desc *rx_descs;
    struct admtek_tx_desc *tx_descs;
    dma_addr_t rx_descs_dma;
    dma_addr_t tx_descs_dma;

    struct sk_buff **rx_skbs;
    struct sk_buff **tx_skbs;
    struct tx_head *tx_heads;

    unsigned cur_rx;
    unsigned cur_tx, dirty_tx;

    struct napi_struct napi;
    struct net_device *dum_dev;
    struct admtek_port *devs[PORT_COUNT];
    struct admtek_port *admtek_devs[ADMTEK_PORT_COUNT];
    struct admtek_port *icplus_devs[ICPLUS_PORT_COUNT];
    unsigned master_port;

    unsigned tx_desc_per_port;
    unsigned tx_desc_count;

    spinlock_t lock;
    spinlock_t linklock;

    unsigned link_status;
    unsigned open_admtek_devs;
    unsigned open_icplus_devs;
};
static struct admtek_switch sw;

extern unsigned char mips_mac_address[6];

/*
static void admtek_dump_macs(void) {
    unsigned st0;
    unsigned st1;
    printk("ADMTEK macs:\n");
    ADMTEK_REG(Srch_cmd_REG) = BIT(0);
    udelay(100);
    while (1) {
	while (1) {
	    if (!(ADMTEK_REG(Srch_cmd_REG) & 3)) break;
	}
	while (1) {
	    udelay(100);
	    st0 = ADMTEK_REG(ADDR_st0_REG);
	    if (st0 & (BIT(0) | BIT(1))) break;
	}
	if (st0 & BIT(1)) break;
	st1 = ADMTEK_REG(ADDR_st1_REG);

	printk("filter: %d vlan: %d %d port: %d age: %d %02x:%02x:%02x:%02x:%02x:%02x\n",
	       (st0 >> 2) & 1,
	       (st0 >> 6) & 1,
	       (st0 >> 3) & 0x7,
	       (st0 >> 7) & 0x3F,
	       (st0 >> 13) & 0x7,
	       (st0 >> 16) & 0xFF,
	       (st0 >> 24) & 0xFF,
	       (st1 >> 0) & 0xFF,
	       (st1 >> 8) & 0xFF,
	       (st1 >> 16) & 0xFF,
	       (st1 >> 24) & 0xFF
	    );

	ADMTEK_REG(Srch_cmd_REG) = BIT(1);
	udelay(100);
    }
}
*/

static void admtek_dump(void) {
//    unsigned i;

    printk("CPUp_conf:  %08lx\n", ADMTEK_REG(CPUp_conf_REG));
    printk("Port_conf0: %08lx\n", ADMTEK_REG(Port_conf0_REG));
    printk("Port_conf1: %08lx\n", ADMTEK_REG(Port_conf1_REG));
    printk("Port_conf2: %08lx\n", ADMTEK_REG(Port_conf2_REG));
    printk("VLAN_GI:    %08lx\n", ADMTEK_REG(VLAN_G1_REG));
    printk("VLAN_GII:   %08lx\n", ADMTEK_REG(VLAN_G2_REG));
    printk("BW_cntl0:   %08lx\n", ADMTEK_REG(BW_cntl0_REG));
    printk("BW_cntl1:   %08lx\n", ADMTEK_REG(BW_cntl1_REG));

/*
    printk("empty_cnt: %08lx\n", ADMTEK_REG(Empty_cnt));
    printk("working: %lx %lx %lx %lx\n",
	   ADMTEK_REG(Recv_LWaddr_REG), ADMTEK_REG(Recv_HWaddr_REG),
	   ADMTEK_REG(Send_LWaddr_REG), ADMTEK_REG(Send_HWaddr_REG)
	);
    printk("base:    %lx %lx %lx %lx\n",
	   ADMTEK_REG(Recv_LBaddr_REG), ADMTEK_REG(Recv_HBaddr_REG),
	   ADMTEK_REG(Send_LBaddr_REG), ADMTEK_REG(Send_HBaddr_REG)
	);

    printk("cur_rx %d, cur_tx %u, dirty_tx %u\n", sw.cur_rx, sw.cur_tx,
           sw.dirty_tx);
    printk("mask 0x%lx, status 0x%lx\n",
           ADMTEK_REG(SW_Int_mask_REG),
           ADMTEK_REG(SW_Int_st_REG));
*/

/*
    for (i = 0; i < 255; i += 4) {
	if (!(i % 16)) printk("\n");
	printk("%08lx ", ADMTEK_REG(i));
    }
    printk("\n");
*/

/*
    for (i = 0; i < RX_DESC_COUNT; ++i) {
	printk("%08lx ", sw.rx_descs[i].buf1);
        if ((i + 1) % 8 == 0) printk("\n");
    }
*/
/*
    for (i = 0; i < RX_DESC_COUNT; ++i) {
	printk("rx %08lx %lx %08lx %lx   %08lx %08lx %08lx\n",
	       sw.rx_descs[i].buf1,
	       sw.rx_descs[i].buf2,
	       sw.rx_descs[i].len,
	       sw.rx_descs[i].status,
	       sw.rx_skbs[i],
	       sw.rx_skbs[i] ? sw.rx_skbs[i]->len : 0,
	       sw.rx_skbs[i] ? (sw.rx_skbs[i]->end - sw.rx_skbs[i]->head) : 0
	    );
    }
*/
/*
    for (i = 0; i < sw.tx_desc_count; ++i) {
	printk("tx %08lx %08lx %08lx %08lx   %08lx\n",
	       sw.tx_descs[i].buf1,
	       sw.tx_descs[i].buf2,
	       sw.tx_descs[i].len,
	       sw.tx_descs[i].ctrl,
	       sw.tx_skbs[i] ? sw.tx_skbs[i]->data : 0
	    );
    }
*/
}

static void admtek_rx_fixup(void) {
    unsigned i;

    for (i = 0; i < RX_DESC_COUNT; ++i) {
	unsigned b = DESC_OWN;
	if (i == RX_DESC_COUNT - 1) b |= DESC_RING_END;

	if (sw.rx_skbs[i]) {
	    sw.rx_descs[i].buf2 = 0;
	    sw.rx_descs[i].len = RX_BUF_SIZE;
	    sw.rx_descs[i].status = 0;
	    sw.rx_descs[i].buf1 =
		((unsigned long)sw.rx_skbs[i]->data & DESC_ADDR_MASK) | b;
	}
	else {
	    sw.rx_descs[i].buf1 = b;
	}
    }
}

static void admtek_init_descs(void) {
    admtek_rx_fixup();

    memset(sw.tx_descs, 0, sizeof(struct admtek_tx_desc) * sw.tx_desc_count);
    sw.tx_descs[sw.tx_desc_count - 1].buf1 |= DESC_RING_END;
    memset(sw.tx_skbs, 0, sizeof(struct sk_buff *) * sw.tx_desc_count);

    sw.cur_rx = 0;
    sw.cur_tx = 0;
    sw.dirty_tx = 0;
}

static void admtek_stop_engine(void) {
    unsigned i;
    printk("admtek_stop_engine\n");
//    admtek_dump();

    icplus_stop_engine();
    ADMTEK_REG(SW_Int_mask_REG) |= SWITCH_INT_MASK; 
    ADMTEK_REG(Port_conf0_REG) |= SW_DISABLE_PORT_MASK; 
    ADMTEK_REG(Port_conf2_REG) = 0x2 | BIT(3);
//    ADMTEK_REG(Boot_done_REG) = 1;
//    ADMTEK_REG(PHY_cntl2_REG) &= ~(0x1F << 20);
//    ADMTEK_REG(SWReset_REG) = 1;
//    ADMTEK_REG(PHY_cntl2_REG) = (1 << 31 | 0x1F << 20 | 0x1F << 25 |
//				 0x1F << 10 | 0x1F << 5 | 0x1F << 0);

    for (i = 0; i < sw.tx_desc_count; ++i) {
	if (sw.tx_skbs[i]) {
	    dev_kfree_skb(sw.tx_skbs[i]);
	    sw.tx_skbs[i] = NULL;
	}
    }

    for (i = 0; i < PORT_COUNT; ++i) {
	if (!sw.devs[i]) continue;
	sw.devs[i]->in_transmit = 0;
    }
}

static void admtek_clear_mac(unsigned char *addr) {
    unsigned reg;

    ADMTEK_REG(MAC_wt1_REG) =
	(addr[5] << 24) | (addr[4] << 16) | (addr[3] << 8) | addr[2];
    reg = (addr[1] << 24) | (addr[0] << 16);
    reg |= SW_MAC_WRITE;

    ADMTEK_REG(MAC_wt0_REG) = reg;

    while (!(ADMTEK_REG(MAC_wt0_REG) & SW_MAC_WRITE_DONE));
}

static void admtek_init_engine(void) {
    printk("admtek_init_engine\n");

    // remove mac address set by bios
    admtek_clear_mac(mips_mac_address);

    admtek_stop_engine();
    admtek_init_descs();

    ADMTEK_REG(0x78) = 0xE8DC1818;
    ADMTEK_REG(0x64) = 0x700;

    icplus_init_engine();

    // initialize engine
    ADMTEK_REG(CPUp_conf_REG) =
	SW_CPU_PORT_DISABLE | SW_PADING_CRC | SW_BRIDGE_MODE;
    ADMTEK_REG(Port_conf0_REG) = SW_DISABLE_PORT_MASK | 0x3F << 8;

    ADMTEK_REG(SW_Int_mask_REG) |= SWITCH_INT_MASK;
    ADMTEK_REG(SW_Int_st_REG) |= SWITCH_INT_MASK;
    ADMTEK_REG(Send_LBaddr_REG) = sw.tx_descs_dma;
    ADMTEK_REG(Send_HBaddr_REG) = 0;
    ADMTEK_REG(Recv_LBaddr_REG) = sw.rx_descs_dma;
    ADMTEK_REG(Recv_HBaddr_REG) = 0;
    ADMTEK_REG(CPUp_conf_REG) &= ~SW_CPU_PORT_DISABLE;

    ADMTEK_REG(SW_Int_mask_REG) &=
	~(INT_MASK | PORT_STATUS_CHANGE_INT);
}

static int admtek_rx(int budget) {
    struct admtek_rx_desc *desc;
    unsigned num;
    struct net_device *dev = NULL;
    struct admtek_port *port;
    unsigned len;
    struct sk_buff *skb;
    int works = 0;

    while (1) {
	desc = sw.rx_descs + sw.cur_rx;
	if (desc->buf1 & DESC_OWN) break;

	num = (desc->status & DESC_RX_PORT_MASK) >> DESC_RX_PORT_SHIFT;
	len = ((desc->status & DESC_LEN_MASK) >> DESC_LEN_SHIFT) - 4;

	port = sw.admtek_devs[num];

//	printk("rx: %08lx, p%u, len: %u\n", desc->status, num, len);
	if (!len || len > RX_BUF_SIZE || desc->status & DESC_RX_CSUM_ERR) {
	    printk("admtek: rx error\n");
	    if (port) {
		port->dev->stats.rx_errors++;
		port->dev->stats.rx_length_errors++;
	    }
	    goto next;
	}

	skb = sw.rx_skbs[sw.cur_rx];
	if (!skb) goto next;
	skb_put(skb, len);

	if (!port) {
	    if (num == sw.master_port) {
//		printk("port %02x %02x%02x\n",
//		       skb->data[13], skb->data[14], skb->data[15]);
//		int i;
		num = ffs(*(skb->data + 13)) - 1;
		*(skb->data + 13) = 0;
		if (num >= ICPLUS_PORT_COUNT) {
		    printk("invalid icplus port %x\n",
			   (unsigned char)*(skb->data + 13));
		    goto next;
		}
		port = sw.icplus_devs[num];

//		printk("packet from port %d %d\n", num, len);
//		for (i = 0; i < 20; ++i) {
//		    printk("%02x ", (unsigned char)*(skb->data + i));
//		}
//		printk("\n");

		if (skb->data[14] == 0xff && skb->data[15] == 0xff) {
		    memmove(skb->data + 4, skb->data, 12);
		    skb_pull(skb, 4);
		}
	    }
	}
	if (!port) {
	    printk("admtek: no device\n");
	    goto next;
	}
	if (port->master) port = port->master;
	dev = port->dev;

	if (skb) {
	    sw.rx_skbs[sw.cur_rx] = NULL;
	    if (netif_running(dev)) {
		skb->dev = dev;
		skb->protocol = eth_type_trans(skb, dev);
		skb->ip_summed = CHECKSUM_UNNECESSARY;

		// only place we need to flush cache
		dma_cache_wback_inv((unsigned long)skb->data, len);

		dev->last_rx = jiffies;
		port->dev->stats.rx_packets++;
		port->dev->stats.rx_bytes += len;
		netif_receive_skb(skb);
		++works;
	    }
	    else {
		dev_kfree_skb_any(skb);
	    }
	}

	next:
	if (!sw.rx_skbs[sw.cur_rx]) {
	    skb = netdev_alloc_skb(dev, RX_BUF_SIZE + 16);
	    if (skb) skb_reserve(skb, 2);
	    else {
		printk("admtek: alloc rx skb failed\n");
	    }
	    sw.rx_skbs[sw.cur_rx] = skb;
	}

	if (sw.rx_skbs[sw.cur_rx]) {
	    skb = sw.rx_skbs[sw.cur_rx];
	    desc->buf2 = 0;
	    desc->status = 0;
	    desc->len = RX_BUF_SIZE;
	    desc->buf1 = (desc->buf1 & DESC_RING_END) | DESC_OWN
		| (((unsigned long)skb->data) & DESC_ADDR_MASK);
	}
	else {
	    desc->buf2 = 0;
	    desc->status = 0;
	    desc->len = 0;
	    desc->buf1 = (desc->buf1 & DESC_RING_END) | DESC_OWN;
	}

	++sw.cur_rx;
	if (sw.cur_rx >= RX_DESC_COUNT) sw.cur_rx = 0;
	if (works >= budget) break;
    }
    return works;
}

static void admtek_tx(void) {
    struct admtek_tx_desc *desc;
    struct admtek_port *port;

    while (1) {
	desc = sw.tx_descs + sw.dirty_tx;
	if (sw.cur_tx == sw.dirty_tx) break;
	if (desc->buf1 & DESC_OWN) break;

//	printk("tx done %x\n", desc->ctrl);
	port = netdev_priv(sw.tx_skbs[sw.dirty_tx]->dev);
	port->dev->stats.tx_packets++;
	port->dev->stats.tx_bytes += sw.tx_skbs[sw.dirty_tx]->len;

	if (port->in_transmit >= sw.tx_desc_per_port) {
	    if (netif_running(port->dev)) {
		if (!timer_pending(&port->pause_timer)) {
		    netif_wake_queue(port->dev);
		}
	    }
	}
	--port->in_transmit;

	dev_kfree_skb(sw.tx_skbs[sw.dirty_tx]);
	sw.tx_skbs[sw.dirty_tx] = NULL;

	++sw.dirty_tx;
	if (sw.dirty_tx >= sw.tx_desc_count) sw.dirty_tx = 0;
    }
}

static int admtek_poll(struct napi_struct *napi, int budget)
{
    int work_done;

    ADMTEK_REG(SW_Int_st_REG) = INT_MASK;

    spin_lock(&sw.lock);

    admtek_tx();

    spin_unlock(&sw.lock);

    work_done = admtek_rx(budget);

    if (work_done >= budget)
        return work_done;

    if ((ADMTEK_REG(SW_Int_st_REG) & INT_MASK) != 0) {
        // new event already pending, stay on poll list
        return budget;
    }

    napi_complete(napi);
    ADMTEK_REG(SW_Int_mask_REG) &= ~INT_MASK;

    return work_done;
}

static irqreturn_t admtek_interrupt(int irq, void *dev_id)
{
    unsigned long status = ADMTEK_REG(SW_Int_st_REG);
//    printk("irq: %08x\n", status);

    if (status & INT_MASK) {
        napi_schedule(&sw.napi);
        ADMTEK_REG(SW_Int_mask_REG) |= INT_MASK;
    }

    if (status & PORT_STATUS_CHANGE_INT) {
	unsigned new_status, i;

        spin_lock(&sw.linklock);

        new_status = ADMTEK_REG(PHY_st_REG);

 	for (i = 0; i < PORT_COUNT; ++i) {
	    unsigned pm;
	    struct net_device *dev;
	    if (!sw.devs[i] || !ADMTEK_PORT(sw.devs[i])) continue;
	    if (sw.devs[i]->master_count) continue;
	    dev = sw.devs[i]->dev;
	    if (!netif_running(dev)) continue;
	    pm = 1 << sw.devs[i]->num;
	    if ((new_status & pm) != (sw.link_status & pm)) {
		if (new_status & pm) {
		    printk("admtek: link up for %s\n", dev->name);
		    netif_carrier_on(dev);
 		}
 		else {
		    printk("admtek: link down for %s\n", dev->name);
		    netif_carrier_off(dev);
 		}
 	    }
 	}

	sw.link_status = new_status;

        spin_unlock(&sw.linklock);

        ADMTEK_REG(SW_Int_st_REG) = PORT_STATUS_CHANGE_INT;
    }

    return IRQ_RETVAL(1);
}

static int admtek_open(struct net_device *dev) {
    struct admtek_port *port = netdev_priv(dev);
    unsigned long flags;
//    printk("admtek_open: %s\n", dev->name);

    if (!sw.open_admtek_devs) {
	admtek_init_engine();
	dev_open(sw.dum_dev);
    }
    ++sw.open_admtek_devs;

    spin_lock_irqsave(&sw.linklock, flags);

//    admtek_set_mac(-1, port->num, dev->dev_addr);
//    admtek_dump_macs();

    if (!port->master_count) {
	if ((ADMTEK_REG(PHY_st_REG) & (1 << port->num))) {
	    printk("admtek: initial link up for %s\n", dev->name);
	    netif_carrier_on(dev);
	    sw.link_status |= (1 << port->num);
	}
	else {
	    printk("admtek: initial link down for %s\n", dev->name);
	    netif_carrier_off(dev);
	    sw.link_status &= ~(1 << port->num);
	}
    }
    else {
	netif_carrier_on(dev);
    }

    spin_unlock_irqrestore(&sw.linklock, flags);

    // enable port
    ADMTEK_REG(Port_conf0_REG) &= ~(1 << port->num); 

    netif_start_queue(dev);
    return 0;
}

static int admtek_stop(struct net_device *dev) {
    struct admtek_port *port = netdev_priv(dev);

//    admtek_dump_macs();

    del_timer_sync(&port->pause_timer);
    netif_stop_queue(dev);

    // disable port
    ADMTEK_REG(Port_conf0_REG) |= (1 << port->num);

    --sw.open_admtek_devs;
    if (!sw.open_admtek_devs) {
	dev_close(sw.dum_dev);
	admtek_stop_engine();
    }

    return 0;
}

static void admtek_pause_timer(unsigned long data) {
    struct net_device *dev = (struct net_device *)data;
    struct admtek_port *port = netdev_priv(dev);
    spin_lock(&sw.lock);

    if ((ADMTEK_REG(Empty_cnt) & 0x1FF) < EMPTY_THRESH) {
//	printk("%lu: continue pause queue: %s, %u\n",
//	       jiffies, dev->name, port->in_transmit);
	netif_stop_queue(dev);

	port->pause_timer.expires = jiffies + 1;
	add_timer(&port->pause_timer);
    }
    else {
//	printk("%lu: resume queue: %s, %u\n",
//	       jiffies, dev->name, port->in_transmit);
	netif_wake_queue(dev);
    }

    spin_unlock(&sw.lock);
}

static int admtek_hard_start_xmit(struct sk_buff *skb,
				  struct net_device *dev) {
    struct admtek_port *port = netdev_priv(dev);
    struct admtek_tx_desc *desc;
    unsigned len;
    int vlan = -1;
//    printk("admtek_hard_start_xmit: %s\n", dev->name);

    spin_lock(&sw.lock);

    sw.tx_skbs[sw.cur_tx] = skb;

    len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;
    desc = sw.tx_descs + sw.cur_tx;

    desc->ctrl = len << DESC_LEN_SHIFT;
    if (port->master) vlan = port->master->num;
    if (port->master_count) vlan = port->num;
    if (vlan < 0) {
	desc->ctrl |= 1 << (port->num + 8);
    }
    else {
	desc->ctrl |= 1 << vlan;
    }
//    printk("xmit desc ctrl %08lx %s\n", desc->ctrl, dev->name);

    desc->len = skb->len;
    desc->buf2 = 0;
    desc->buf1 = (desc->buf1 & DESC_RING_END) | 
	((unsigned long)skb->data & DESC_ADDR_MASK) | DESC_OWN;

    ADMTEK_REG(Send_trig_REG) = 1;

    dev->trans_start = jiffies;

    ++sw.cur_tx;
    if (sw.cur_tx >= sw.tx_desc_count) sw.cur_tx = 0;

    port->in_transmit++;
    if (port->in_transmit >= sw.tx_desc_per_port) {
//	printk("%lu: stop queue: %s, %u\n",
//	       jiffies, dev->name, port->in_transmit);
	netif_stop_queue(dev);
	goto ret;
    }

    if ((ADMTEK_REG(Empty_cnt) & 0x1FF) < EMPTY_THRESH) {
//	printk("%lu: pause queue: %s, %u\n",
//	       jiffies, dev->name, port->in_transmit);
	netif_stop_queue(dev);

	port->pause_timer.expires = jiffies + 1;
	add_timer(&port->pause_timer);
	goto ret;
    }
ret:
    spin_unlock(&sw.lock);
    return 0;
}

static void admtek_get_drvinfo (struct net_device *dev,
				struct ethtool_drvinfo *info) {
    struct admtek_port *port = netdev_priv(dev);
    strcpy(info->driver, "admtek");
    strcpy(info->version, "1.0");
    sprintf(info->bus_info, "00:00.0 admtek%d", port->num);
}

static int admtek_get_settings(struct net_device *dev,
			       struct ethtool_cmd *cmd) {
    struct admtek_port *port = netdev_priv(dev);
    unsigned long reg;
    unsigned long flags;
    spin_lock_irqsave(&sw.linklock, flags);

    // inspired from mii.c
    cmd->supported =
	(SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
	 SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
	 SUPPORTED_Autoneg | SUPPORTED_TP | SUPPORTED_MII);
    cmd->port = PORT_MII;
    cmd->transceiver = XCVR_INTERNAL;
    cmd->phy_address = 0;

    cmd->advertising = ADVERTISED_TP | ADVERTISED_MII;
    cmd->advertising |= ADVERTISED_10baseT_Half;
    cmd->advertising |= ADVERTISED_10baseT_Full;
    cmd->advertising |= ADVERTISED_100baseT_Half;
    cmd->advertising |= ADVERTISED_100baseT_Full;

    reg = ADMTEK_REG(PHY_cntl2_REG);
    if ((reg & (1 << port->num))) {
	cmd->advertising |= ADVERTISED_Autoneg;
	cmd->autoneg = AUTONEG_ENABLE;
    }
    else cmd->autoneg = AUTONEG_DISABLE;

    reg = ADMTEK_REG(PHY_st_REG);
    if ((reg & ((1 << 8) << port->num))) cmd->speed = SPEED_100;
    else cmd->speed = SPEED_10;
    if ((reg & ((1 << 16) << port->num))) cmd->duplex = DUPLEX_FULL;
    else cmd->duplex = DUPLEX_HALF;

    spin_unlock_irqrestore(&sw.linklock, flags);
    return 0;
}

static int admtek_set_settings(struct net_device *dev,
			       struct ethtool_cmd *cmd) {
    struct admtek_port *port = netdev_priv(dev);
    unsigned long flags;
    spin_lock_irqsave(&sw.linklock, flags);
//    printk("%s: admtek_set_settings autoneg:%u speed:%u duplex:%u\n",
//	    dev->name, cmd->autoneg, cmd->speed, cmd->duplex);

    if (cmd->autoneg == AUTONEG_ENABLE) {
	ADMTEK_REG(PHY_cntl2_REG) |= (1 << port->num);
	ADMTEK_REG(PHY_cntl2_REG) |= ((1 << 5) << port->num);
	ADMTEK_REG(PHY_cntl2_REG) |= ((1 << 10) << port->num);
    } else {
	ADMTEK_REG(PHY_cntl2_REG) &= ~(1 << port->num);

	if (cmd->speed == SPEED_100) {
	    ADMTEK_REG(PHY_cntl2_REG) |= ((1 << 5) << port->num);
	}
	else {
	    ADMTEK_REG(PHY_cntl2_REG) &= ~((1 << 5) << port->num);
	}

	if (cmd->duplex == DUPLEX_FULL) {
	    ADMTEK_REG(PHY_cntl2_REG) |= ((1 << 10) << port->num);
	}
	else {
	    ADMTEK_REG(PHY_cntl2_REG) &= ~((1 << 10) << port->num);
	}
    }

    spin_unlock_irqrestore(&sw.linklock, flags);
    return 0;
}

static u32 admtek_get_link(struct net_device *dev) {
    struct admtek_port *port = netdev_priv(dev);
    int link = !!(sw.link_status & (1 << port->num));
    if (!port->link && link) {
	if (port->master) port->master->master_links++;
    }
    if (port->link && !link) {
	if (port->master) port->master->master_links--;
    }
    port->link = link;
    return port->link + port->master_links;
}


static struct ethtool_ops admtek_ethtool_ops = {
    .get_drvinfo	= admtek_get_drvinfo,
    .get_settings	= admtek_get_settings,
    .set_settings	= admtek_set_settings,
    .get_link		= admtek_get_link,
};

static int icplus_get_settings(struct net_device *dev,
			       struct ethtool_cmd *cmd) {
    struct admtek_port *port = netdev_priv(dev);
    return mii_ethtool_gset(&port->mii_info, cmd);
}

static int icplus_set_settings(struct net_device *dev,
				 struct ethtool_cmd *cmd) {
    struct admtek_port *port = netdev_priv(dev);
    return mii_ethtool_sset(&port->mii_info, cmd);
}

static u32 icplus_get_link(struct net_device *dev) {
    struct admtek_port *port = netdev_priv(dev);
    int link = mii_link_ok(&port->mii_info);
    if (!port->link && link) {
	if (port->master) port->master->master_links++;
    }
    if (port->link && !link) {
	if (port->master) port->master->master_links--;
    }
    port->link = link;
    return port->link + port->master_links;
}
 
static struct ethtool_ops icplus_ethtool_ops = {
    .get_drvinfo  = admtek_get_drvinfo,
    .get_settings = icplus_get_settings,
    .set_settings = icplus_set_settings,
    .get_link     = icplus_get_link,
};



static void admtek_tx_timeout(struct net_device *dev) {
    unsigned i;
    printk("admtek: tx timeout %s\n", dev->name);

    admtek_dump();
    spin_lock(&sw.lock);
    admtek_init_engine();

    // BUG: need to setup switch stuff aswell, but never have seen it recover
    // from tx_timeout so its dead by now anyway
    for (i = 0; i < PORT_COUNT; ++i) {
	struct admtek_port *port = sw.devs[i];
	if (!port || !ADMTEK_PORT(port) || !netif_running(port->dev)) continue;
	ADMTEK_REG(Port_conf0_REG) &= ~(1 << port->num); 
    }

    spin_unlock(&sw.lock);
    admtek_dump();
}

static int admtek_change_mtu(struct net_device *dev, int new_mtu) {
    if (new_mtu < 68 || new_mtu > dev->l2mtu) return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}

static int admtek_set_mac_address(struct net_device *dev, void *p) {
    struct sockaddr *addr = p;

    if (!is_valid_ether_addr(addr->sa_data))
	return -EADDRNOTAVAIL;
    memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
    return 0;
}

static int admtek_get_type(struct admtek_port *port) {
    if (port->num < 100) {
	return SWITCH_ADMTEK;
    }
    return SWITCH_ICPLUS175C;
}

static int admtek_get_switch(struct admtek_port *port) {
    if (port->num < 100) {
	return 0;
    }
    return 1;
}

static void admtek_set_bandwidth_tx(struct admtek_port *port, int bw) {
    if (port->num < 100) {
	if (port->num < 4) {
	    ADMTEK_REG(BW_cntl0_REG) &= ~(0x7 << (port->num * 8));
	    ADMTEK_REG(BW_cntl0_REG) |= bw << (port->num * 8);
	}
	else {
	    ADMTEK_REG(BW_cntl1_REG) &= ~(0x7);
	    ADMTEK_REG(BW_cntl1_REG) |= bw;
	}
    }
    else {
	unsigned reg = (port->num - 100) / 2;
	unsigned off = (port->num - 100) % 2 * 8 + 4;
	unsigned val = icplus_read_reg(31, reg);
	val &= ~(0x7 << off);
	val |= bw << off;
	icplus_write_reg(31, reg, val);
    }
}

static void admtek_set_bandwidth_rx(struct admtek_port *port, int bw) {
    if (port->num < 100) {
	if (port->num < 4) {
	    ADMTEK_REG(BW_cntl0_REG) &= ~(0x7 << (port->num * 8 + 4));
	    ADMTEK_REG(BW_cntl0_REG) |= bw << (port->num * 8 + 4);
	}
	else {
	    ADMTEK_REG(BW_cntl1_REG) &= ~(0x7 << 4);
	    ADMTEK_REG(BW_cntl1_REG) |= bw << 4;
	}
    }
    else {
	unsigned reg = (port->num - 100) / 2;
	unsigned off = (port->num - 100) % 2 * 8;
	unsigned val = icplus_read_reg(31, reg);
	val &= ~(0x7 << off);
	val |= bw << off;
	icplus_write_reg(31, reg, val);
    }
}

static void admtek_set_vlan(struct admtek_port *port, int vlan, int enable) {
    int num = port->num;
    struct admtek_port *master = sw.admtek_devs[vlan];
    int reg = vlan < 4 ? VLAN_G1_REG : VLAN_G2_REG;
    int off = vlan % 4 * 8;

//    printk("admtek set vlan %d %d %d\n", num, vlan, enable);
    if (enable) {
	ADMTEK_REG(reg) |= BIT(num) << off;

	ADMTEK_REG(Port_conf1_REG) &= ~BIT(num);
	ADMTEK_REG(Port_conf1_REG) &= ~BIT(num + 6);
	ADMTEK_REG(Port_conf1_REG) |= BIT(num + 20);

	netif_carrier_on(port->dev);

	master->master_ports |= BIT(num);
    }
    else {
	ADMTEK_REG(reg) &= ~(BIT(num) << off);

	ADMTEK_REG(Port_conf1_REG) |= BIT(num);
	ADMTEK_REG(Port_conf1_REG) |= BIT(num + 6);
	ADMTEK_REG(Port_conf1_REG) &= ~BIT(num + 20);

	master->master_ports &= ~BIT(num);
    }
}

static void icplus_set_vlan(struct admtek_port *port, int vlan, int enable) {
    int num = port->num - 100;
    int ports = BIT(vlan);
    int mask;
    int i;
    int val;
    struct admtek_port *master = sw.icplus_devs[vlan];
//    printk("%s master %s %d\n", port->dev->name, master->dev->name, enable);
    for (i = 0; i < ICPLUS_PORT_COUNT; ++i) {
	if (!sw.icplus_devs[i]) continue;
	if (!sw.icplus_devs[i]->master) continue;
	if (sw.icplus_devs[i]->master->num - 100 == vlan) {
	    ports |= BIT(sw.icplus_devs[i]->num - 100);
	}
    }

    if (enable) {
	mask = ports | BIT(7);
    }
    else {
	mask = (ports & ~BIT(num)) | BIT(7);
    }

    for (i = 0; i < 5; ++i) {
	int reg = 19 + i / 2;
	int off = i % 2 ? 0 : 8;
	if (!(BIT(i) & ports)) continue;
	val = icplus_read_reg(29, reg);
	val &= ~(0xFF << off);
	val |= mask << off;
	icplus_write_reg(29, reg, val);
    }

    if (!enable) ports &= ~BIT(num);
    master->master_ports = ports;
    if (master->master_ports == BIT(vlan)) master->master_ports = 0;

    val = icplus_read_reg(30, 16);
    val &= ~BIT(num + 8);
    val |= enable << (num + 8);
    icplus_write_reg(30, 16, val);
}

static void admtek_special_set_vlan(struct admtek_port *port, int vlan,
				    int enable) {
    if (vlan < 100) {
	admtek_set_vlan(port, vlan, enable);
    }
    else {
	vlan -= 100;
	icplus_set_vlan(port, vlan, enable);
    }
}

struct admtek_port *admtek_find(unsigned ifindex) {
    int i;
    for (i = 0; i < PORT_COUNT; ++i) {
	if (!sw.devs[i]) continue;
	if (sw.devs[i]->dev->ifindex == ifindex) return sw.devs[i];
    }
    return NULL;
}

static void admtek_set_master(struct admtek_port *port, int m) {
    struct admtek_port *master = admtek_find(m);
    int vlan;

    if (port->master) {
	port->master->master_links -= port->link;
	vlan = port->master->num;
	--port->master->master_count;
	if (!port->master->master_count) {
	    admtek_special_set_vlan(port->master, vlan, 0);
	}
	admtek_special_set_vlan(port, vlan, 0);
    }

    port->master = master;

    if (port->master) {
	port->master->master_links += port->link;
	vlan = port->master->num;
	++port->master->master_count;
	if (port->master->master_count == 1) {
	    admtek_special_set_vlan(port->master, vlan, 1);
	}
	admtek_special_set_vlan(port, vlan, 1);
    }

//    icplus_dump_regs();
//    admtek_dump();
}

#define ifr_ivalue  ifr_ifru.ifru_ivalue

static int admtek_ioctl(struct net_device *dev, struct ifreq *rq, int cmd) {
    struct admtek_port *port = netdev_priv(dev);

    if (cmd >= SIOCGTYPE && cmd < SIOCSWICHMAX) {
	struct switch_params params;
        if (copy_from_user(&params, rq->ifr_ifru.ifru_data, sizeof(params))) {
	    return -EINVAL;
	}
	switch (cmd) {
	case SIOCGTYPE:
	    params.data = admtek_get_type(port);
	    break;
	case SIOCGSWITCH:
	    params.data = admtek_get_switch(port);
	    break;
	case SIOCSTXBW:
	    admtek_set_bandwidth_tx(port, params.data);
	    break;
	case SIOCSRXBW:
	    admtek_set_bandwidth_rx(port, params.data);
	    break;
	case SIOCSMASTER:
	    admtek_set_master(port, params.data);
	    break;
	case SIOCGPORT:
	    params.data = port->num;
	    break;
	}
	if (copy_to_user(rq->ifr_ifru.ifru_data, &params, sizeof(params))) {
	    return -EINVAL;
	}
	return 0;
    }

    if (!netif_running(dev))
	return -EINVAL;

    if (!port->mii_info.dev) return -ENOTSUPP;

    return generic_mii_ioctl(&port->mii_info, if_mii(rq), cmd, NULL);
}

static struct net_device_ops admtek_dummy_netdev_ops = {
};

static void admtek_dum_setup(struct net_device *dev) {
    dev->type = ARPHRD_VOID;
    dev->mtu = 1500;
    dev->tx_queue_len = 1000;
    dev->flags = IFF_NOARP;
    netif_napi_add(dev, &sw.napi, admtek_poll, 64);
}

static struct net_device *admtek_setup_dummy(void) {
    struct net_device *dev = alloc_netdev(0, "admtek", admtek_dum_setup);
    if (!dev) return NULL;

    dev->netdev_ops = &admtek_dummy_netdev_ops;

    if (register_netdev(dev) < 0) {
	printk("admtek: register dummy failed\n");
	kfree(dev);
	return NULL;
    }

    return dev;
}




static int icplus_open(struct net_device *dev) {
//    printk("icplus: open %s\n", dev->name);
    if (!sw.open_icplus_devs) {
	if (!sw.open_admtek_devs) {
	    admtek_init_engine();
	    dev_open(sw.dum_dev);
	}
	++sw.open_admtek_devs;

	// enable port
	ADMTEK_REG(Port_conf0_REG) &= ~(1 << sw.master_port); 
    }
    ++sw.open_icplus_devs;

    netif_start_queue(dev);
    return 0;
}

static int icplus_stop(struct net_device *dev) {
//    printk("icplus: stop %s\n", dev->name);
    netif_stop_queue(dev);

    --sw.open_icplus_devs;
    if (!sw.open_icplus_devs) {
	// disable port
	ADMTEK_REG(Port_conf0_REG) |= (1 << sw.master_port);

	--sw.open_admtek_devs;
	if (!sw.open_admtek_devs) {
	    dev_close(sw.dum_dev);
	    admtek_stop_engine();
	}
    }

    return 0;
}

static int icplus_hard_start_xmit(struct sk_buff *skb,
				  struct net_device *dev) {
    struct admtek_port *port = netdev_priv(dev);
    struct tx_head *head = &sw.tx_heads[sw.cur_tx];
    struct admtek_tx_desc *desc;
    unsigned len;

//    printk("icplus: xmit %s %d %d\n", dev->name,
//	   skb_headroom(skb), skb_tailroom(skb));

    if (skb_padto(skb, ETH_ZLEN))
	return 0;

    spin_lock(&sw.lock);

    memcpy(head->macs, skb->data, 12);
    head->vlan0 = 0x81;
    head->vlan1 = BIT(port->num - 100);
    if (port->master_ports) {
	head->vlan1 = port->master_ports;
    }
    head->vlan2 = 0x00;
    head->vlan3 = port->num - 100;

    desc = sw.tx_descs + sw.cur_tx;
    sw.tx_skbs[sw.cur_tx] = skb;

    len = (skb->len + 4) < ETH_ZLEN ? ETH_ZLEN : (skb->len + 4);
    desc->ctrl = (len << DESC_LEN_SHIFT) | (1 << (sw.master_port + 8));
    desc->len = 16;
    desc->buf2 = (1 << 31) | (unsigned long)(skb->data + 12);
    desc->buf1 = (desc->buf1 & DESC_RING_END) | (unsigned long)head | DESC_OWN;
//    printk("tx desc %lx %lx %lx %lx %lx\n",
//	   desc->buf1, desc->buf2, desc->len, desc->ctrl,
//	   ADMTEK_REG(PHY_st_REG)
//	);

    ADMTEK_REG(Send_trig_REG) = 1;

    dev->trans_start = jiffies;

    ++sw.cur_tx;
    if (sw.cur_tx >= sw.tx_desc_count) sw.cur_tx = 0;

    port->in_transmit++;
    if (port->in_transmit >= sw.tx_desc_per_port) {
//	printk("%lu: stop queue: %s, %u\n",
//	       jiffies, dev->name, port->in_transmit);
	netif_stop_queue(dev);
    }

    spin_unlock(&sw.lock);
    return 0;
}

static void icplus_write_phy(struct net_device *dev, int phy, int reg,
			     int value) {
    spin_lock(&sw.linklock);
    icplus_write_reg(phy, reg, value);
    spin_unlock(&sw.linklock);
}

static int icplus_read_phy(struct net_device *dev, int phy, int reg) {
    int value;
    spin_lock(&sw.linklock);
    value = icplus_read_reg(phy, reg);
    spin_unlock(&sw.linklock);
    return value;
}

static const struct net_device_ops admtek_netdev_ops = {
	.ndo_open		= admtek_open,
	.ndo_stop		= admtek_stop,
	.ndo_start_xmit		= admtek_hard_start_xmit,
	.ndo_change_mtu		= admtek_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_tx_timeout		= admtek_tx_timeout,
	.ndo_do_ioctl		= admtek_ioctl,
	.ndo_set_mac_address	= admtek_set_mac_address,
};

static const struct net_device_ops icplus_netdev_ops = {
	.ndo_open		= icplus_open,
	.ndo_stop		= icplus_stop,
	.ndo_start_xmit		= icplus_hard_start_xmit,
	.ndo_change_mtu		= admtek_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_tx_timeout		= admtek_tx_timeout,
	.ndo_do_ioctl		= admtek_ioctl,
	.ndo_set_mac_address	= admtek_set_mac_address,
};


static int admtek_init_dev(struct net_device *dev, unsigned num,
			   unsigned mac_idx) {
    struct admtek_port *port = netdev_priv(dev);
    int ret = 0;

    memcpy(dev->dev_addr, mips_mac_address, 6);

    if (dev->dev_addr[5] + mac_idx > 255) ++dev->dev_addr[4];
    dev->dev_addr[5] += mac_idx;

    dev->watchdog_timeo = HZ;
    dev->irq = SW_IRQ;

    if (num < 100) {
	dev->netdev_ops = &admtek_netdev_ops;
	dev->ethtool_ops = &admtek_ethtool_ops;
	dev->l2mtu = 1518;
	sw.admtek_devs[num] = port;
    }
    else {
	dev->netdev_ops = &icplus_netdev_ops;
	dev->ethtool_ops = &icplus_ethtool_ops;
	dev->l2mtu = 1514;

	port->mii_info.dev = dev;
	port->mii_info.mdio_read = &icplus_read_phy;
	port->mii_info.mdio_write = &icplus_write_phy;
	port->mii_info.phy_id = num - 100;
	port->mii_info.phy_id_mask = 0x1f;
	port->mii_info.reg_num_mask = 0x1f;
	sw.icplus_devs[num - 100] = port;
    }

    ret = register_netdev(dev);
    if (ret) {
	printk("admtek: register_netdev failed\n");
	return ret;
    }
//    printk("admtek: %s -> %d\n", dev->name, num);

    port->dev = dev;
    port->num = num;

    init_timer(&port->pause_timer);
    port->pause_timer.data = (unsigned long)dev;
    port->pause_timer.function = &admtek_pause_timer;

    return ret;
}

static void admtek_cleanup_dev(struct admtek_port *port) {
    if (port->dev) unregister_netdev(port->dev);
    kfree(port->dev);
}

static void admtek_cleanup(void) {
    unsigned i;
    unsigned c;

    ADMTEK_REG(SW_Int_mask_REG) |= SWITCH_INT_MASK; 

    free_irq(SW_IRQ, &sw);

    dev_close(sw.dum_dev);
    unregister_netdev(sw.dum_dev);
    kfree(sw.dum_dev);
    for (i = 0; i < PORT_COUNT; ++i) {
	if (sw.devs[i]) admtek_cleanup_dev(sw.devs[i]);
    }
    if (sw.tx_skbs) {
	for (i = 0; i < sw.tx_desc_count; ++i) {
	    if (sw.tx_skbs[i]) dev_kfree_skb(sw.tx_skbs[i]);
	}
	kfree(sw.tx_skbs);
    }
    if (sw.tx_heads) {
	kfree(sw.tx_heads);
    }
    if (sw.tx_descs) {
	c = sw.tx_desc_count;
	pci_free_consistent(NULL, sizeof(struct admtek_rx_desc) * c,
			    sw.tx_descs, sw.tx_descs_dma);
    }
    if (sw.rx_skbs) {
	for (i = 0; i < RX_DESC_COUNT; ++i) {
	    if (sw.rx_skbs[i]) dev_kfree_skb(sw.rx_skbs[i]);
	}
	kfree(sw.rx_skbs);
    }
    if (sw.rx_descs) {
	c = RX_DESC_COUNT;
	pci_free_consistent(NULL, sizeof(struct admtek_rx_desc) * c,
			    sw.rx_descs, sw.rx_descs_dma);
    }
}

static int __init admtek_probe(struct platform_device *pdev) {
    int *port_map = (int *) pdev->dev.platform_data;
    unsigned i;

    printk("ADMTEK ethernet switch driver\n");

    sw.tx_desc_per_port = 4;
    sw.tx_desc_count = ((sw.tx_desc_per_port + 1) * PORT_COUNT);

    if (request_irq(SW_IRQ, admtek_interrupt, IRQF_SHARED | IRQF_DISABLED,
		    "ethernet switch", &sw)) {
	printk("admtek: request irq failed\n");
	goto err_out;
    }

    // alloc buffers
    sw.rx_descs = pci_alloc_consistent(NULL, sizeof(struct admtek_rx_desc)
				       * RX_DESC_COUNT, &sw.rx_descs_dma);
    if (!sw.rx_descs) {
	printk("admtek: alloc rx descriptors failed\n");
	goto err_out;
    }

    sw.rx_skbs = kmalloc(sizeof(struct sk_buff *) * RX_DESC_COUNT, GFP_KERNEL);
    if (!sw.rx_skbs) {
	printk("admtek: alloc rx skb ptr buf failed\n");
	goto err_out;
    }
    memset(sw.rx_skbs, 0, sizeof(struct sk_buff *) * RX_DESC_COUNT);

    sw.tx_descs = pci_alloc_consistent(NULL, sizeof(struct admtek_tx_desc) *
				       sw.tx_desc_count, &sw.tx_descs_dma);
    if (!sw.tx_descs) {
	printk("admtek: alloc tx descriptors failed\n");
	goto err_out;
    }

    sw.tx_skbs = kmalloc(sizeof(struct sk_buff *) * sw.tx_desc_count,
			 GFP_KERNEL);
    if (!sw.tx_skbs) {
	printk("admtek: alloc tx skb ptr buf failed\n");
	goto err_out;
    }
    memset(sw.tx_skbs, 0, sizeof(struct sk_buff *) * sw.tx_desc_count);

    sw.tx_heads = kmalloc(sizeof(struct tx_head) * sw.tx_desc_count,
			  GFP_KERNEL | GFP_DMA);
    if (!sw.tx_heads) {
	printk("admtek: alloc tx head buf failed\n");
	goto err_out;
    }
    memset(sw.tx_heads, 0, sizeof(struct tx_head) * sw.tx_desc_count);

    spin_lock_init(&sw.lock);
    spin_lock_init(&sw.linklock);

    ADMTEK_REG(Port_conf1_REG) = 0x00000FFF;
    ADMTEK_REG(VLAN_G1_REG) = 0;
    ADMTEK_REG(VLAN_G2_REG) = 0;

    sw.dum_dev = admtek_setup_dummy();
    if (!sw.dum_dev) {
	printk("admtek: dummy device setup failed\n");
	goto err_out;
    }

    for (i = 0; i < RX_DESC_COUNT; ++i) {
	sw.rx_skbs[i] = netdev_alloc_skb(sw.dum_dev, RX_BUF_SIZE + 16);
	if (!sw.rx_skbs[i]) {
	    printk("admtek: alloc rx skb failed\n");
	    goto err_out;
	}
	skb_reserve(sw.rx_skbs[i], 2);
    }

    napi_enable(&sw.napi);
    sw.master_port = port_map[0];
    for (i = 0; i < PORT_COUNT; ++i) {
	struct net_device *dev;
	int p = port_map[i + 1];
	if (p < 0) break;

	dev = alloc_etherdev(sizeof(struct admtek_port));
	if (dev == NULL) {
	    printk("admtek: alloc_etherdev failed for port #%u\n", p);
	    goto err_out;
	}

	if (admtek_init_dev(dev, p, i)) {
	    printk("admtek: init failed for port #%u\n", p);
	    goto err_out;
	}
	sw.devs[i] = netdev_priv(dev);
    }

    return 0;
err_out:
    admtek_cleanup();
    return -ENODEV;
}

static int __exit admtek_remove(struct platform_device *pdev) {
    admtek_cleanup();
    return 0;
}

static struct platform_driver admtek_driver = {
	.probe	= admtek_probe,
	.remove = admtek_remove,
	.driver	= {
		.name	= "admtek",
		.owner	= THIS_MODULE,
	}
};

static int __init admtek_init(void) {
	return platform_driver_register(&admtek_driver);
}

static void __exit admtek_exit(void) {
	platform_driver_unregister(&admtek_driver);
}

module_init(admtek_init);
module_exit(admtek_exit);
