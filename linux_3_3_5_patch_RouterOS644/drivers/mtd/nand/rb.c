#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/crc32.h>

#define BOOTER_PART_SIZE	256 * 1024
#define KERNEL_SMALL_PART_SIZE	4 * 1024 * 1024
#define KERNEL_PART_SIZE	6 * 1024 * 1024

static unsigned booter_size = 0;
static struct mtd_info rmtd;
static struct mtd_info *mtdp = &rmtd;
static int rmtd_valid = 0;

int rb_big_boot_partition = 0;

static unsigned boot_part_size = 0;

static int __init set_boot_part_size(char *s) {
    boot_part_size = simple_strtoul(s + 1, NULL, 0);
    return 1;
}
__setup("boot_part_size", set_boot_part_size);

static unsigned parts = 1;
static int __init set_parts(char *s) {
    parts = simple_strtoul(s + 1, NULL, 0);
    if (parts < 1) parts = 1;
    if (parts > 8) parts = 8;
    return 0;
}
__setup("parts", set_parts);

#if defined(MIPSEL) && !defined(CONFIG_SMP)
static int nand_is_bad = 0;

static int check_nand(struct mtd_info *mtd) {
	unsigned char ids[4] = { 0, 0, 0, 0 };
	size_t retlen;

	mtd_read_fact_prot_reg(mtd, 0, 4, &retlen, ids);

	nand_is_bad = (ids[0] == 0xad &&
		       ids[1] == 0xf1 &&
		       ids[2] == 0x80 &&
		       ids[3] == 0x1d) ? 1 : 0;
	return 0;
}

int is_nand_bad(void) {
	return nand_is_bad;
}
EXPORT_SYMBOL(is_nand_bad);

static int nand_supports_backup(struct mtd_info *mtd) {
	unsigned ofs;

	for (ofs = mtd->size / 2; ofs < mtd->size; ofs += mtd->erasesize) {
		struct mtd_oob_ops ops;
		uint32_t oobdata;

		ops.ooblen = 4;
		ops.oobbuf = (uint8_t *)&oobdata;
		ops.ooboffs = BACKUP_4xFF_OFFSET;
		ops.datbuf = NULL;
		ops.mode = MTD_OPS_RAW;

		if (mtd->block_isbad(mtd, ofs)) {
			continue;
		}
		if (mtd->read_oob(mtd, ofs, &ops)) {
			printk(KERN_INFO "nand backup disabled - "
			       "read error at block %u\n",
			       ofs / mtd->erasesize);
			return 0;
		}
		if (oobdata != 0xffffffff) {
			printk(KERN_INFO "nand backup disabled - "
			       "backup area not empty at block %u\n",
			       ofs / mtd->erasesize);
			return 0;
		}
	}

	printk(KERN_INFO "nand backup enabled\n");
	return 1;
}

int nand_enable_backup(struct mtd_info *mtd_part) {
	struct nand_chip *chip = rmtd.priv;
	if (!rmtd_valid) return 0;
	if (chip->backup_offset) return 0;
	if (!is_nand_bad()) return 0;
	if (!nand_supports_backup(&rmtd)) return 0;

	chip->backup_offset = rmtd.size / 2;
	parts = 1;
	if (mtd_part && mtd_part != &rmtd) {
		// reduce mtd partition size
		mtd_part->size -= chip->backup_offset;
	}
	return 1;
}
EXPORT_SYMBOL(nand_enable_backup);
#endif

static int rb_nand_scan(struct mtd_info *mtd, int maxchips)
{
	struct nand_chip *chip = mtd->priv;
	int ret;

	ret = nand_scan_ident(mtd, maxchips, NULL);
	chip->bbt_options &= ~NAND_BBT_SCANLASTPAGE;
	
	if (!ret)
		ret = nand_scan_tail(mtd);
	return ret;
}

static unsigned get_boot_part_size(void)
{
#ifndef __tile__
	if (boot_part_size) {
	    return boot_part_size;
	} else if (rb_big_boot_partition) {
	    return KERNEL_PART_SIZE;
	} else {
	    return KERNEL_SMALL_PART_SIZE;
	}
#else
	return 16 * 1024 * 1024;
#endif
}

static int register_partitions(void)
{
	unsigned offset = 0;
	unsigned bootsize = get_boot_part_size();
	unsigned nandsize = mtdp->size;
	unsigned eraseshift = ffs(mtdp->erasesize) - 1;
	unsigned partsize;
	unsigned i;
#ifdef MIPSEL
	struct nand_chip *chip = mtdp->priv;
	if (chip->backup_offset) nandsize = chip->backup_offset;
#endif
	partsize = ((nandsize >> eraseshift) / parts) << eraseshift;

	for (i = 0; i < parts; ++i) {
	    char name[80];
	    unsigned bsize = bootsize;
	    int ret;

	    if (i == 0) {
		offset += booter_size;
		bsize -= booter_size;
	    }

	    snprintf(name, sizeof(name), "RouterBoard NAND %u Boot", i + 1);
	    ret = mtd_add_partition(mtdp, name, offset, bsize);
	    if (ret != 0) return ret;
	    offset += bsize;

	    snprintf(name, sizeof(name), "RouterBoard NAND %u Main", i + 1);
	    ret = mtd_add_partition(mtdp, name,
				    offset, partsize - bootsize);
	    offset += partsize - bootsize;
	    if (ret != 0) return ret;
	}

	if (booter_size) {
	    return mtd_add_partition(mtdp, "RouterBoot NAND Booter",
				     0, booter_size);
	}

	return 0;
}

static ssize_t partitions_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", parts);
}

static ssize_t partitions_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count)
{
	extern int del_mtd_partitions(struct mtd_info *);
	unsigned long p;
	int ret;

	if (strict_strtoul(buf, 0, &p) || p < 1 || p > 8)
		return -EINVAL;

	parts = p;
	
	ret = del_mtd_partitions(mtdp);
	register_partitions();

	return count;
}

static struct kobj_attribute partitions_attr = 
    __ATTR(partitions, 0644, partitions_show, partitions_store);

int rb_nand_probe(struct nand_chip *nand, int booter)
{
	mtdp = &rmtd;
	memset(&rmtd, 0, sizeof(rmtd));

	nand->ecc.mode = NAND_ECC_SOFT;
	nand->chip_delay = 25;
	nand->options |= NAND_NO_AUTOINCR;
	rmtd.priv = nand;

	if (rb_nand_scan(&rmtd, 1) && rb_nand_scan(&rmtd, 1)
	    && rb_nand_scan(&rmtd, 1)  && rb_nand_scan(&rmtd, 1)) {
		printk("RBxxx nand device not found\n");
		return -ENXIO;
	}
	rmtd_valid = 1;

	if (sysfs_create_file(kernel_kobj, &partitions_attr.attr)) {
	    printk("ERROR: could not create sys/kernel/partitions");
	}

#if defined(MIPSEL) && !defined(CONFIG_SMP)
	check_nand(&rmtd);
	nand_enable_backup(NULL);
#endif
	booter_size = booter ? max(BOOTER_PART_SIZE, (int)rmtd.erasesize) : 0;
	return register_partitions();
}

int rb_nand_register_partitions(struct mtd_info *mtd) {
	mtdp = mtd;
	rb_big_boot_partition = 1;
	if (sysfs_create_file(kernel_kobj, &partitions_attr.attr)) {
	    printk("ERROR: could not create sys/kernel/partitions");
	}
	return register_partitions();
}
