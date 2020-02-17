#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mtd/nand.h>
#include <linux/of_platform.h>
#include <linux/spinlock.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <asm/io.h>

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static struct nand_chip rnand;
extern spinlock_t localbus_lock;

/* ------------- support for NAND flash control machine ------------- */

#define CFG_LOCAL_OFFSET	0x00005000
#define CFG_LOCAL_SIZE		0x00000100

#define LOCAL_OPTION_REG1	0x0000000c
#define   LO_FCM_PGS			0x00000400

#define LOCAL_BUS_MDR		0x00000088
#define LOCAL_BUS_LSOR		0x00000090
#define LOCAL_BUS_LTESR		0x000000b0
#define   LTESR_FCT			0x40000000
#define   LTESR_CC			0x00000001
#define LOCAL_BUS_LTEDR		0x000000b4
#define LOCAL_BUS_LTEIR		0x000000b8
#define LOCAL_BUS_LTEATR	0x000000bc

#define LOCAL_BUS_FMR		     0x0e0
#define   LBUS_FMR_CWTO_MASK		0x0000f000
#define   LBUS_FMR_CWTO_MAX		0x0000f000
#define   LBUS_FMR_BOOT			0x00000800
#define   LBUS_FMR_AL_MASK		0x00000030
#define   LBUS_FMR_AL_SHIFT		4
#define   LBUS_FMR_AL_BASE		2
#define   LBUS_FMR_OP_MASK		0x00000003
#define   LBUS_FMR_OP_READ		0x00000002
#define   LBUS_FMR_OP_WRITE		0x00000003
#define LOCAL_BUS_FIR		     0x0e4
#define   LBUS_FIR_NOP			       0x0
#define   LBUS_FIR_CA			       0x1
#define   LBUS_FIR_PA			       0x2
#define   LBUS_FIR_UA			       0x3
#define   LBUS_FIR_CM0			       0x4
#define   LBUS_FIR_CM1			       0x5
#define   LBUS_FIR_CM2			       0x6
#define   LBUS_FIR_CM3			       0x7
#define   LBUS_FIR_WB			       0x8
#define   LBUS_FIR_WS			       0x9
#define   LBUS_FIR_RB			       0xa
#define   LBUS_FIR_RS			       0xb
#define   LBUS_FIR_CW0			       0xc
#define   LBUS_FIR_CW1			       0xd
#define   LBUS_FIR_RBW			       0xe
#define   LBUS_FIR_RSW			       0xf
#define LOCAL_BUS_FCR		     0x0e8
#define   LBUS_FCR_CMD0_SHIFT			24
#define   LBUS_FCR_CMD1_SHIFT			16
#define   LBUS_FCR_CMD2_SHIFT			 8
#define   LBUS_FCR_CMD3_SHIFT			 0
#define LOCAL_BUS_FBAR		     0x0ec
#define LOCAL_BUS_FPAR		     0x0f0
#define   LBUS_FPAR_OFS_0000		0x00000000
#define   LBUS_FPAR_OFS_1000		0x00002000
#define LOCAL_BUS_FBCR		     0x0f4

enum NandCmd {
    NCMD_END = 1,
    NCMD_NOP = 2,
    NCMD_BIT_ALE = (1 << 28),
    NCMD_BIT_CLE = (1 << 29),
    NCMD_MASK = (0x3f << 24),

    NCMD_CLE = NCMD_BIT_CLE,
    NCMD_ALE = (1 << 24) | NCMD_BIT_ALE,
    NCMD_ALE_OFS = (2 << 24) | NCMD_BIT_ALE,
    NCMD_ALE_PAGE = (3 << 24) | NCMD_BIT_ALE,

    NCMD_WAIT_RDY = (1 << 24),
    NCMD_READ = (2 << 24),
    NCMD_WRITE = (3 << 24),
    NCMD_ALLOW_WRITE = (4 << 24),
    NCMD_READ_BYTE = (5 << 24),
};

#define NAND_BANK	1

struct rbfcm_info {
	unsigned long lbus;
	unsigned long ndata;
	unsigned pi_shift;
	unsigned pi_bits;
	volatile uint8_t *ptr;
	unsigned cle_prefix;
	unsigned cle_prefix_len;
};

static inline uint32_t read32(unsigned long addr) {
	return in_be32((unsigned *)addr);
}
static inline void write32(unsigned long addr, uint32_t val) {
	out_be32((unsigned *)addr, val);
}

static void nand_setcfg(struct rbfcm_info *info,
			unsigned page_size, unsigned pages_in_nand) {
	unsigned ale_page_bytes = (pages_in_nand + 7) / 8;
	unsigned cfg;

	printk("nand_setcfg: page_size %u, pages_in_nand %u\n",
	       page_size, pages_in_nand);
	cfg = read32(info->lbus + LOCAL_OPTION_REG1);
	if (page_size == 9) {
		cfg &= ~LO_FCM_PGS;
		info->pi_shift = 10;
		info->pi_bits = 5;
	}
	else {
		cfg |= LO_FCM_PGS;
		info->pi_shift = 12;
		info->pi_bits = 6;
	}
	write32(info->lbus + LOCAL_OPTION_REG1, cfg);

	cfg = read32(info->lbus + LOCAL_BUS_FMR);
        cfg &= ~LBUS_FMR_AL_MASK;
	cfg |= (ale_page_bytes - LBUS_FMR_AL_BASE) << LBUS_FMR_AL_SHIFT;
	write32(info->lbus + LOCAL_BUS_FMR, cfg);
}

static uint8_t rbfcm_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct rbfcm_info *info = chip->priv;
	return *(info->ptr++);
}

static void rbfcm_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct rbfcm_info *info = chip->priv;
	memcpy(buf, (const void *)info->ptr, len);
	info->ptr += len;
}

static int rbfcm_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct rbfcm_info *info = chip->priv;
	if (memcmp(buf, (const void *)info->ptr, len) != 0)
		return -EFAULT;
	info->ptr += len;
	return 0;
}

static void rbfcm_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len) {
	struct nand_chip *chip = mtd->priv;
	struct rbfcm_info *info = chip->priv;
	memcpy((void*)info->ptr, buf, len);
	info->ptr += len;
}

static int nand_cmd_setup(struct rbfcm_info *info, const unsigned *ncmd,
			  unsigned len) {
	unsigned fcr = 0;
	unsigned fcr_idx = 0;
	unsigned fir = 0;
	unsigned fir_idx = 0;
	unsigned fpar = 0;
	unsigned fbar = 0;
	unsigned mdr = 0;
	unsigned mdr_idx = 0;
	unsigned fmr;
	int waitRdy = 0;

	info->ptr = NULL;
	for ( ; *ncmd != NCMD_END; ++ncmd) {
		unsigned cmd = *ncmd & NCMD_MASK;
		unsigned data = *ncmd & 0x00ffffff;
		unsigned fir_cmd = LBUS_FIR_NOP;
		if (*ncmd == NCMD_NOP) continue;
		switch (cmd) {
		case NCMD_CLE:
			if (waitRdy && fcr_idx <= 1) {
				fir_cmd = LBUS_FIR_CW0 + fcr_idx;
				waitRdy = 0;
			}
			else {
				fir_cmd = LBUS_FIR_CM0 + fcr_idx;
			}
			++fcr_idx;
			fcr |= ((data & 0xff) << (8 * (4 - fcr_idx)));
			break;
		case NCMD_ALE:
			mdr |= ((data & 0xff) << (8 * mdr_idx));
			++mdr_idx;
			fir_cmd = LBUS_FIR_UA;
			break;
		case NCMD_ALE_OFS:
			fpar |= (data & ((1 << info->pi_shift) - 1));
			fir_cmd = LBUS_FIR_CA;
			break;
		case NCMD_ALE_PAGE:
			fpar |= (data & ((1 << info->pi_bits) - 1))
					    << info->pi_shift;
			fbar = data >> info->pi_bits;
			fir_cmd = LBUS_FIR_PA;
			break;
		case NCMD_WAIT_RDY:
			waitRdy = 1;
			break;
		case NCMD_READ:
			if (waitRdy) {
				fir_cmd = LBUS_FIR_RBW;
				waitRdy = 0;
			}
			else {
				fir_cmd = LBUS_FIR_RB;
			}
			info->ptr = (void *)(info->ndata | (fpar & 0x1fff));
			break;
		case NCMD_READ_BYTE:
			if (waitRdy) {
				fir_cmd = LBUS_FIR_RSW;
				waitRdy = 0;
			}
			else {
				fir_cmd = LBUS_FIR_RS;
			}
			break;
		case NCMD_WRITE:
			fir_cmd = LBUS_FIR_WB;
			info->ptr = (void *)(info->ndata | (fpar & 0x1fff));
			break;
		case NCMD_ALLOW_WRITE:
			/* FIXME, currently writes are always enabled */
			break;
		}
		if (fir_cmd != LBUS_FIR_NOP) {
			++fir_idx;
			fir |= (fir_cmd << (4 * (8 - fir_idx)));
			if (fir_idx > 8) {
				printk("NAND CMD ERROR: too many actions within command\n");
				return 0;
			}
		}
		if (waitRdy && cmd != NCMD_WAIT_RDY) {
			printk("NAND CMD ERROR: wait ready before cmd %x not implemented\n",
			      cmd);
			waitRdy = 0;
		}
	}
	/* wait for previous command to finish */
	do {
		fmr = read32(info->lbus + LOCAL_BUS_FMR);
	} while (fmr & LBUS_FMR_OP_MASK);

	/* clear status register */
	write32(info->lbus + LOCAL_BUS_LTESR, 0xffffffff);
	write32(info->lbus + LOCAL_BUS_LTEATR, 0);

	/* setup registers for CMD */
	write32(info->lbus + LOCAL_BUS_FBCR, len);
	write32(info->lbus + LOCAL_BUS_MDR, mdr);
	write32(info->lbus + LOCAL_BUS_FBAR, fbar);
	write32(info->lbus + LOCAL_BUS_FPAR, fpar);
	write32(info->lbus + LOCAL_BUS_FCR, fcr);
	write32(info->lbus + LOCAL_BUS_FIR, fir);
	write32(info->lbus + LOCAL_BUS_FMR, fmr);
	return 1;
}

static int nand_cmd_run(struct rbfcm_info *info) {
	unsigned ltesr;
	unsigned long flags;

	spin_lock_irqsave(&localbus_lock, flags);
	write32(info->lbus + LOCAL_BUS_FMR,
		read32(info->lbus + LOCAL_BUS_FMR) | LBUS_FMR_OP_WRITE);
	read32(info->lbus + LOCAL_BUS_FMR);	/* flush */

	/* start special command */
	write32(info->lbus + LOCAL_BUS_LSOR, NAND_BANK);
	read32(info->lbus + LOCAL_BUS_LSOR);

	/* wait for command to finish */
	do {
		ltesr = read32(info->lbus + LOCAL_BUS_LTESR);
	} while (ltesr == 0);

	// FIXME: error processing
	spin_unlock_irqrestore(&localbus_lock, flags);

	return !(ltesr & LTESR_FCT);
}

static unsigned nand_cmd_get_read_byte(struct rbfcm_info *info) {
	return read32(info->lbus + LOCAL_BUS_MDR) & 0xff;
}

static int nand_cmd_run_status(struct rbfcm_info *info, const char *op_str) {
	unsigned status;
	if (!nand_cmd_run(info)) {
		printk(KERN_ERR "rbfcm: %s cmd failed\n", op_str);
		return 0;
	}
	status = nand_cmd_get_read_byte(info);
	if ((status & (NAND_STATUS_FAIL | NAND_STATUS_READY))
			!= NAND_STATUS_READY) {
		printk(KERN_ERR "rbfcm: %s failed, status %x\n",
		       op_str, status);
		return 0;
	}
	return 1;
}

static int nand_read(struct rbfcm_info *info,
		     const unsigned *ncmd, unsigned len) {
	if (!nand_cmd_setup(info, ncmd, len)) return 0;
	return nand_cmd_run(info);
}

static int nand_cmd_status(struct rbfcm_info *info, const unsigned *ncmd,
			   const char *op_str) {
	if (!nand_cmd_setup(info, ncmd, 0)) return 0;
	return nand_cmd_run_status(info, op_str);
}

static void rbfcm_select_chip(struct mtd_info *mtd, int chipnr) {
}

static void rbfcm_read(struct mtd_info *mtd, struct rbfcm_info *info,
		       int offset, int page, int len) {
	unsigned ncmd_read[] = {
		NCMD_WAIT_RDY,
		NCMD_NOP,
		NCMD_NOP,
		NCMD_NOP,
		NCMD_NOP,
		NCMD_CLE | NAND_CMD_READ0,
		NCMD_ALE_OFS | offset,
		NCMD_ALE_PAGE | page,
		NCMD_CLE | NAND_CMD_READSTART,	/* large page only */
		NCMD_WAIT_RDY | 1000,		/* 1ms */
		NCMD_READ,
		NCMD_END
	};
	if (mtd->oobsize == 16) {
		if (offset >= 512) {
			ncmd_read[5] |= NAND_CMD_READOOB;
		}
		else if (offset >= 256) {
			ncmd_read[5] |= NAND_CMD_READ1;
		}
		ncmd_read[8] = NCMD_NOP;
	}
	for (; info->cle_prefix_len != 0; --info->cle_prefix_len) {
		int shift = (info->cle_prefix_len - 1) * 8;
		unsigned clcmd = (info->cle_prefix >> shift) & 0xff;
		ncmd_read[5 - info->cle_prefix_len] = NCMD_CLE | clcmd;
	}
	nand_read(info, ncmd_read, len);
}

static void rbfcm_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	register struct nand_chip *chip = mtd->priv;
	struct rbfcm_info *info = chip->priv;

	if (ctrl & NAND_CLE) {
		info->cle_prefix = (info->cle_prefix << 8) | (cmd & 0xff);
		if (info->cle_prefix_len < 2) ++info->cle_prefix_len;
	}
}

static void rbfcm_command(struct mtd_info *mtd, unsigned int command,
			  int column, int page)
{
	register struct nand_chip *chip = mtd->priv;
	struct rbfcm_info *info = chip->priv;

	if (command == NAND_CMD_RESET) {
		unsigned ncmd_reset[] = {
			NCMD_CLE | NAND_CMD_RESET,
			NCMD_WAIT_RDY | 10000,		/* 10ms */
			NCMD_CLE | NAND_CMD_STATUS,
			NCMD_READ_BYTE,
			NCMD_END
		};
		nand_cmd_status(info, ncmd_reset, "reset");
		return;
	}
	if (command == NAND_CMD_READID) {
		unsigned ncmd_readid[] = {
			NCMD_CLE | NAND_CMD_READID,
			NCMD_ALE | column,
			NCMD_READ,
			NCMD_END
		};
		nand_read(info, ncmd_readid, 5);
		return;
	}
	if (command == NAND_CMD_PARAM) {
		unsigned ncmd_param[] = {
			NCMD_CLE | NAND_CMD_PARAM,
			NCMD_ALE | column,
			NCMD_WAIT_RDY | 1000,		/* 1ms */
			NCMD_READ,
			NCMD_END
		};
		nand_read(info, ncmd_param, 2112);
		return;
	}
	if (command == NAND_CMD_ONFI_READUID) {
		unsigned ncmd_readuid[] = {
			NCMD_CLE | NAND_CMD_ONFI_READUID,
			NCMD_ALE | column,
			NCMD_WAIT_RDY | 1000,		/* 1ms */
			NCMD_READ,
			NCMD_END
		};
		nand_read(info, ncmd_readuid, 512);
		return;
	}
	if (command == NAND_CMD_STATUS) {
		unsigned ncmd_status[] = {
			NCMD_ALLOW_WRITE,
			NCMD_CLE | NAND_CMD_STATUS,
			NCMD_READ,
			NCMD_END
		};
		nand_read(info, ncmd_status, 1);
		return;
	}
	if (info->pi_shift == 0) {
		/* TODO: initialize settings somewhere else */
		nand_setcfg(info, rnand.page_shift,
			    rnand.chip_shift - rnand.page_shift);
	}
	if (command == NAND_CMD_ERASE1) {
		unsigned ncmd_erase[] = {
			NCMD_ALLOW_WRITE,
			NCMD_WAIT_RDY,
			NCMD_CLE | NAND_CMD_ERASE1,
			NCMD_ALE_PAGE | page,
			NCMD_CLE | NAND_CMD_ERASE2,
			NCMD_CLE | NAND_CMD_STATUS,
			NCMD_WAIT_RDY | 100000,		/* 100ms */
			NCMD_READ_BYTE,
			NCMD_END
		};
		nand_cmd_status(info, ncmd_erase, "erase");
		return;
	}
	if (command == NAND_CMD_ERASE2) {
		/* erase is already done */
		return;
	}
	if (command == NAND_CMD_READOOB) {
		unsigned len = (column != 0) ? 1 : mtd->oobsize;
		rbfcm_read(mtd, info, column + mtd->writesize, page, len);
		return;
	}
	if (command == NAND_CMD_READ0) {
		rbfcm_read(mtd, info, column, page,
			   mtd->writesize + mtd->oobsize - column);
		return;
	}

	printk(KERN_ERR "rbfcm: unknown command %x\n", command);
}

static int rbfcm_write_setup(struct mtd_info *mtd, struct nand_chip *chip,
			     int offset, int page, int len) {
	struct rbfcm_info *info = chip->priv;
	unsigned ncmd_write[] = {
		NCMD_ALLOW_WRITE,
		NCMD_WAIT_RDY,
		NCMD_CLE | NAND_CMD_READ0,	/* on small block device only */
		NCMD_CLE | NAND_CMD_SEQIN,
		NCMD_ALE_OFS | offset,
		NCMD_ALE_PAGE | page,
		NCMD_WRITE,
		NCMD_CLE | NAND_CMD_PAGEPROG,
		NCMD_CLE | NAND_CMD_STATUS,
		NCMD_WAIT_RDY | 10000,		/* 10ms */
		NCMD_READ_BYTE,
		NCMD_END
	};

	if (mtd->oobsize != 16) {
		ncmd_write[2] = NCMD_NOP;
	}
	else if (offset >= 512) {
		ncmd_write[2] |= NAND_CMD_READOOB;
	}
	else if (offset >= 256) {
		ncmd_write[2] |= NAND_CMD_READ1;
	}

	return nand_cmd_setup(info, ncmd_write, len);
}

static int rbfcm_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			    const uint8_t *buf, int page, int cached, int raw)
{
	struct rbfcm_info *info = chip->priv;

	if (!rbfcm_write_setup(mtd, chip, 0, page,
			       mtd->writesize + mtd->oobsize))
		return -EIO;

	if (unlikely(raw))
		chip->ecc.write_page_raw(mtd, chip, buf);
	else
		chip->ecc.write_page(mtd, chip, buf);

	if (!nand_cmd_run_status(info, "write page"))
		return -EIO;

	return 0;
}

static int rbfcm_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
			   int page)
{
	struct rbfcm_info *info = chip->priv;
	const uint8_t *buf = chip->oob_poi;
	int length = mtd->oobsize;

	if (!rbfcm_write_setup(mtd, chip, mtd->writesize, page, length))
		return -EIO;

	rbfcm_write_buf(mtd, buf, length);
	return nand_cmd_run_status(info, "write oob") ? 0 : -EIO;
}

static int rbfcm_wait_rdy(struct mtd_info *mtd, struct nand_chip *chip)
{
	/*
	 * write/program command already waits for ready,
	 * just return status read
	 */
	struct rbfcm_info *info = chip->priv;
	return nand_cmd_get_read_byte(info);
}

/* XXX: move nand_prepare() to RouterBOOT ? */
static void nand_prepare(struct rbfcm_info *info) {
	unsigned cfg;

	cfg = read32(info->lbus + LOCAL_OPTION_REG1);
	cfg |= 0x3fe;	/* TODO: optimize timings */
	write32(info->lbus + LOCAL_OPTION_REG1, cfg);

	cfg = read32(info->lbus + LOCAL_BUS_FMR);
	cfg &= ~(LBUS_FMR_OP_MASK | LBUS_FMR_BOOT | LBUS_FMR_CWTO_MASK);
	cfg |= LBUS_FMR_CWTO_MAX;
	write32(info->lbus + LOCAL_BUS_FMR, cfg);

	write32(info->lbus + LOCAL_BUS_LTEDR, 0);
	write32(info->lbus + LOCAL_BUS_LTEIR, 0);
}

static int rbfcm_nand_probe(struct platform_device *pdev)

{
	struct device_node *np;
	struct resource res;
	struct rbfcm_info *info;

	printk("RB_FCM NAND\n");

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		printk("rbfcm error: OOM\n");
		return -1;
	}
	memset(info, 0, sizeof(*info));

	np = of_find_node_by_type(NULL, "soc");
	if (!np) {
		printk("rbfcm error: no soc found\n");
		goto err;
	}
	if (of_address_to_resource(np, 0, &res)) {
		printk("rbfcm error: no reg property for soc\n");
		goto err;
	}
	info->lbus = (unsigned)ioremap_nocache(res.start + CFG_LOCAL_OFFSET,
					       CFG_LOCAL_SIZE);

	if (of_address_to_resource(pdev->dev.of_node, 0, &res)) {
		printk("rbfcm error: no reg property found\n");
		goto err;
	}
	info->ndata = (unsigned)ioremap_nocache(res.start,
						res.end - res.start + 1);

	memset(&rnand, 0, sizeof(rnand));
	rnand.priv = info;

	rnand.read_byte = rbfcm_read_byte;
	rnand.read_buf = rbfcm_read_buf;
	rnand.verify_buf = rbfcm_verify_buf;
	rnand.write_buf = rbfcm_write_buf;

	rnand.select_chip = rbfcm_select_chip;
	rnand.cmd_ctrl = rbfcm_ctrl;
	rnand.cmdfunc = rbfcm_command;
	rnand.waitfunc = rbfcm_wait_rdy;
	rnand.write_page = rbfcm_write_page;
	rnand.ecc.write_oob = rbfcm_write_oob;

	nand_prepare(info);
	return rb_nand_probe(&rnand, 0);

  err:
	kfree(info);
	return -1;
}

static struct of_device_id rbfcm_nand_ids[] = {
	{ .name = "nand_fcm" },
	{}
};

static struct platform_driver rbfcm_nand_driver = {
	.driver	= {
		.name = "rbfcm-nand",
		.owner = THIS_MODULE,
		.of_match_table = rbfcm_nand_ids,
	},
	.probe	= rbfcm_nand_probe,
};

static int __init rbppc_nand_init(void)
{
	return platform_driver_register(&rbfcm_nand_driver);
}

static void __exit rbppc_nand_exit(void)
{
	platform_driver_unregister(&rbfcm_nand_driver);
}
							     
module_init(rbppc_nand_init);
module_exit(rbppc_nand_exit);
