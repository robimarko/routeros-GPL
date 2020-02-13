/*
 * arch/arm/mm/cache-aurora-l2.c - AURORA shared L2 cache controller support
 *
 * Copyright (C) 2008 Marvell Semiconductor
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 * References:
 * - Unified Shared Layer 2 Cache for Armada CP SoC devices,
 *   Document ID MV-S104858-00, Rev. A, October 23 2007.
 */

#include <asm/uaccess.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>
#include <linux/proc_fs.h>

//#include <plat/cache-aurora-l2.h>
#include "cache-aurora-l2.h"
#include <asm/io.h>
//#include <asm/cp15.h>
//#include <mach/smp.h>
//#include "ctrlEnv/mvCtrlEnvSpec.h"
//#include "ctrlEnv/mvCtrlEnvLib.h"
//#include "ctrlEnv/mvSemaphore.h"

/*
 * L2 registers offsets
 */
#define L2_CONTROL		0x100
#define L2_AUX_CONTROL		0x104
#define L2_SYNC			0x700
#define L2_RANGE_BASE		0x710
#define L2_INVALIDATE_PA	0x770
#define L2_INVALIDATE_RANGE	0x774
#define L2_CLEAN_PA		0x7B0
#define L2_CLEAN_RANGE		0x7B4
#define L2_FLUSH_PA		0x7F0
#define L2_FLUSH_RANGE		0x7F4
#define L2_LOCKDOWN_DATA	0x900
#define L2_LOCKDOWN_INSTR	0x904
#define L2_LOCKDOWN_FPU		0x980
#define L2_LOCKDOWN_IO_BRG	0x984

static unsigned int l2rep = L2ACR_REPLACEMENT_TYPE_SEMIPLRU;
static int __init l2rep_setup(char *str)
{
	if (!strcmp(str, "WayRR"))
		l2rep = L2ACR_REPLACEMENT_TYPE_WAYRR;
	else if (!strcmp(str, "LFSR"))
		l2rep = L2ACR_REPLACEMENT_TYPE_LFSR;
	else if (!strcmp(str, "pLRU"))
		l2rep = L2ACR_REPLACEMENT_TYPE_SEMIPLRU;
	else
		return 0;
	return 1;
}
__setup("l2rep=", l2rep_setup);

void __iomem *auroraL2_base = NULL;
bool auroraL2_enable = 0;
#ifdef CONFIG_PROC_FS
static unsigned char *replacement[] = {"WayRR",
				       "LFSR",
				       "semi pLRU",
				       "reserved",
};
static unsigned char *associativity[] ={"reserved",
					"reserved", 
					"reserved", 
					"4-way", 
					"reserved", 
					"reserved", 
					"reserved", 
					"8-way",
					"reserved", 
					"reserved", 
					"reserved", 
					"12-way", 
					"reserved", 
					"reserved", 
					"reserved", 
					"16-way", 
					};
static unsigned char *wsize[]={   "reserved(16KB)", 
                                  "16KB",
                                  "32KB",
                                  "64KB",
                                  "128KB",
                                  "256KB",
                                  "512KB",
                                  "reserved(512KB)"
                              };
static unsigned char *wa_mode[] = {"Requester Attribute", 
                                  "force no write allocate",
                                  "force write allocate",
                                  "reserved"
                              	};
static unsigned char *wbwt_mode[] = {"PageAttribute", 
                                  "force WB",
                                  "force WT",
                                  "reserved"
                              	};



static int proc_auroraL2_info_read(char *page, char **start, off_t off, int count, int *eof,
		    void *data)
{
	char *p = page;
	int len;
    	__u32 aux;

	p += sprintf(p, "AuroraL2 Information:\n");
			 
	aux = readl(auroraL2_base + L2_AUX_CTRL_REG);		 
	p += sprintf(p, "Replacement   : %s\n", replacement[(aux >> L2ACR_REPLACEMENT_OFFSET) & 0x3]);
	p += sprintf(p, "Associativity : %s\n", associativity[(aux >> 13) & 0xf]);
	p += sprintf(p, "Way size      : %s\n", wsize[(aux >> 17) & 0xF]);
	p += sprintf(p, "Data ECC      : %s\n", ((aux >> 20) & 0x1) ? "Enabled" : "Disabled");
	p += sprintf(p, "TAG parity    : %s\n", ((aux >> 21) & 0x1) ? "Enabled" : "Disabled");
 	p += sprintf(p, "Write mode forcing     : %s\n", wbwt_mode[(aux >> 0) & 0x3]);
	p += sprintf(p, "Write allocate forcing : %s\n", wa_mode[(aux >> 23) & 0x3]);
   
	len = (p - page) - off;
	if (len < 0)
		len = 0;

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}

#ifdef CONFIG_CACHE_AURORAL2_EVENT_MONITOR_ENABLE
static unsigned int last_counter[2][2] = {{0,0},{0,0}};

static unsigned char *event_name[]= {    
                                        "Counter Disabled", 
                                        "CastOut",
                                        "DataRdHit",
                                        "DataRdReq",
                                        "DataWrHit",
                                        "DataWrReq",
                                        "DataWTReq",
                                        "InstRdHit",
                                        "InstRdReq",
                                        "MmuRdHit",
                                        "MmuRdReq",
                                        "WriteWAMiss",
                                        "WriteWACLReq",
                                        "WriteWANoCLReq",
                                        "SRAMWr",
                                        "SRAMrRd",
                                        "RMWWrite"
                                        "SpeculativeInstReq"
                                        "SpeculativeInstHit"
                                        "RGFStall"
                                        "EBStall"
                                        "LRBStall"
                                        "Idle"
                                        "Active"
                                    };
static int proc_auroraL2_counter_read(char *page, char **start, off_t off, int count, int *eof,
		    void *data)
{
	char *p = page;
	int len, i, cfg;
    	unsigned int counter[2][2], delta_counter[2][2];

	p += sprintf(p, "AuroraL2 Event Counter Information:\n\n");			 
	p += sprintf(p, "L2_CNTR_CTRL_REG    : %#08x\n", readl(auroraL2_base + L2_CNTR_CTRL_REG));

	for (i = 0; i < L2_MAX_COUNTERS; i++){
		cfg = readl(auroraL2_base + L2_CNTR_CONFIG_REG(i));
		p += sprintf(p, "L2_CNTR%d_CONFIG_REG : %#08x[%s]\n", i, cfg, event_name[(cfg >> 2) & 0x1F]);
	     	counter[i][0] = readl(auroraL2_base + L2_CNTR_VAL_LOW_REG(i));
    		counter[i][1] = readl(auroraL2_base + L2_CNTR_VAL_HIGH_REG(i));
        	delta_counter[i][0] = counter[i][0] - last_counter[i][0];
        	delta_counter[i][1] = counter[i][1] - last_counter[i][1];
	}
    
    	p += sprintf(p, "\n=========================================================================\n");
    	p += sprintf(p, "currnet counter 0 1: %12u%12u     %12u%12u\n",  counter[0][1],  counter[0][0],  counter[1][1], counter[1][0]);
    	p += sprintf(p, "delta   counter 0 1: %12u%12u     %12u%12u\n",  delta_counter[0][1],  delta_counter[0][0],  delta_counter[1][1], delta_counter[1][0]);

	len = (p - page) - off;
	if (len < 0)
		len = 0;

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

    	memcpy((unsigned char *)last_counter, (unsigned char *)counter, sizeof(last_counter));
    
	return len;
}


static int proc_auroraL2_counter_write(struct file *file, const char __user *buffer,
				unsigned long count, void *data)
{
    u8 param[3][32];
    u32 configs[3] = {0};
    u8 *buffer_tmp = kmalloc(count+16, GFP_KERNEL);
    int i, cfg;

    memset(buffer_tmp, 0x0, sizeof(buffer_tmp));

    if(copy_from_user(buffer_tmp, buffer, count))
    {
        if (buffer_tmp)        
            kfree(buffer_tmp);
        return -EFAULT;    
    }

    sscanf(buffer_tmp, "%s %s %s\n",  param[0], param[1], param[2]);

    if (strcmp(param[0], "reset") == 0)
    {
	for (i = 0; i < L2_MAX_COUNTERS; i++){
		/* Stop counters */
		cfg = readl(auroraL2_base + L2_CNTR_CONFIG_REG(i));
		cfg &= ~(0x3F << 2);
        	writel(cfg, auroraL2_base + L2_CNTR_CONFIG_REG(i)); 
	}
        writel(0x101, auroraL2_base + L2_CNTR_CTRL_REG); /* reset counter values */

        memset((unsigned char *)last_counter, 0, sizeof(last_counter));         
        
        goto out;
    }

    for (i = 0; i < 3; i++)
        configs[i] = simple_strtoul(param[i], NULL, 0);             

    writel(configs[0], auroraL2_base + L2_CNTR_CTRL_REG); 
    writel(configs[1], auroraL2_base + L2_CNTR_CONFIG_REG(0)); 
    writel(configs[2], auroraL2_base + L2_CNTR_CONFIG_REG(1)); 	 

out:
   
    if (buffer_tmp)        
        kfree(buffer_tmp);
    
	return count;
}
#endif /* CONFIG_CACHE_TAUROS3_EVENT_MONITOR_ENABLE */
#endif /* CONFIG_PROC_FS */

#define CACHE_LINE_SIZE		32
#define MAX_RANGE_SIZE		1024
#define RANGE_OP

static int l2_wt_override = 0;
#if defined(RANGE_OP) && defined(CONFIG_AURORA_L2_OUTER)
static DEFINE_SPINLOCK(smp_l2cache_lock);

#endif
/*
 * Low-level cache maintenance operations.
 *
 *
 * Cache range operations are initiated by writing the start and
 * end addresses to successive cp15 registers, and process every
 * cache line whose first byte address lies in the inclusive range
 * [start:end-1] (the end address is inclusive).
 *
 * The cache range operations stall the CPU pipeline until completion.
 *
 * The range operations require two successive cp15 writes, in
 * between which we don't want to be preempted.
 */

static inline void cache_sync(void)
{    
    writel(0, auroraL2_base+L2_SYNC);  /* flush L2 write buffer (barrier) */ 
}

#ifdef CONFIG_AURORA_L2_OUTER
inline void l2_clean_pa(unsigned int addr)
{
    	if (!auroraL2_enable)
        	return;        

	writel(addr & ~0x1f, auroraL2_base+L2_CLEAN_PA);
	cache_sync();
}

static inline void l2_inv_pa(unsigned long addr)
{
	if (!auroraL2_enable)
        	return;        

	writel(addr & ~0x1f, auroraL2_base+L2_INVALIDATE_PA);
}

static inline void l2_clean_inv_pa(unsigned long addr)
{
	if (!auroraL2_enable)
        	return;        

	writel(addr & ~0x1f, auroraL2_base+L2_FLUSH_PA);
	cache_sync();
}

void l2_clean_va(unsigned int addr)
{
	l2_clean_pa(__pa(addr));
}

static inline void l2_clean_pa_range(unsigned long start, unsigned long end)
{
	unsigned long flags;

	/*
	 * Make sure 'start' and 'end' reference the same page, as
	 * L2 is PIPT and range operations only do a TLB lookup on
	 * the start address.
	 */
	BUG_ON((start ^ end) & ~(PAGE_SIZE - 1));
#ifdef RANGE_OP
	spin_lock_irqsave(&smp_l2cache_lock, flags);
#ifdef CONFIG_SMP
	writel(start, auroraL2_base+L2_RANGE_BASE + (4 * hard_smp_processor_id()));
#else	
	writel(start, auroraL2_base+L2_RANGE_BASE);
#endif
	writel(end, auroraL2_base+L2_CLEAN_RANGE);
	spin_unlock_irqrestore(&smp_l2cache_lock, flags);
#else
	for(; start <= end; start += CACHE_LINE_SIZE)
		writel(start, auroraL2_base+L2_CLEAN_PA);
#endif
	cache_sync();
}

static inline void l2_flush_pa_range(unsigned long start, unsigned long end)
{
	unsigned long flags;

	/*
	 * Make sure 'start' and 'end' reference the same page, as
	 * L2 is PIPT and range operations only do a TLB lookup on
	 * the start address.
	 */
	BUG_ON((start ^ end) & ~(PAGE_SIZE - 1));
#ifdef RANGE_OP
	spin_lock_irqsave(&smp_l2cache_lock, flags);
#ifdef CONFIG_SMP
	writel(start, auroraL2_base+L2_RANGE_BASE + (4 * hard_smp_processor_id()));
#else
	writel(start, auroraL2_base+L2_RANGE_BASE);
#endif
	writel(end, auroraL2_base+L2_FLUSH_RANGE);
	spin_unlock_irqrestore(&smp_l2cache_lock, flags);
#else
	for ( ; start <= end; start += CACHE_LINE_SIZE)
		writel(start, auroraL2_base+L2_FLUSH_PA);
#endif
	cache_sync();
}

static inline void l2_inv_pa_range(unsigned long start, unsigned long end)
{
	unsigned long flags;

	/*
	 * Make sure 'start' and 'end' reference the same page, as
	 * L2 is PIPT and range operations only do a TLB lookup on
	 * the start address.
	 */
	BUG_ON((start ^ end) & ~(PAGE_SIZE - 1));
#ifdef RANGE_OP
	spin_lock_irqsave(&smp_l2cache_lock, flags);
#ifdef CONFIG_SMP
	writel(start, auroraL2_base+L2_RANGE_BASE + (4 * hard_smp_processor_id()));
#else	
	writel(start, auroraL2_base+L2_RANGE_BASE);
#endif
	writel(end, auroraL2_base+L2_INVALIDATE_RANGE);
	spin_unlock_irqrestore(&smp_l2cache_lock, flags);
#else
	for(; start <= end; start += CACHE_LINE_SIZE)
		writel(start, auroraL2_base+L2_INVALIDATE_PA);
#endif

	cache_sync();
}


/*
 * Linux primitives.
 *
 * Note that the end addresses passed to Linux primitives are
 * noninclusive, while the hardware cache range operations use
 * inclusive start and end addresses.
 */

static inline unsigned long calc_range_end(unsigned long start, unsigned long end)
{
	unsigned long range_end;

	BUG_ON(start & (CACHE_LINE_SIZE - 1));
	BUG_ON(end & (CACHE_LINE_SIZE - 1));

	/*
	 * Try to process all cache lines between 'start' and 'end'.
	 */
	range_end = end;

	/*
	 * Limit the number of cache lines processed at once,
	 * since cache range operations stall the CPU pipeline
	 * until completion.
	 */
	if (range_end > start + MAX_RANGE_SIZE)
		range_end = start + MAX_RANGE_SIZE;

	/*
	 * Cache range operations can't straddle a page boundary.
	 */
	if (range_end > (start | (PAGE_SIZE - 1)) + 1)
		range_end = (start | (PAGE_SIZE - 1)) + 1;

	return range_end;
}

static void aurora_l2_inv_range(unsigned long start, unsigned long end)
{
    	if (!auroraL2_enable)
        	return;        
	/*
	 * Clean and invalidate partial first cache line.
	 */
	if (start & (CACHE_LINE_SIZE - 1)) {
		l2_clean_inv_pa(start & ~(CACHE_LINE_SIZE - 1));
		start = (start | (CACHE_LINE_SIZE - 1)) + 1;
	}

	/*
	 * Clean and invalidate partial last cache line.
	 */
	if (start < end && end & (CACHE_LINE_SIZE - 1)) {
		l2_clean_inv_pa(end & ~(CACHE_LINE_SIZE - 1));
		end &= ~(CACHE_LINE_SIZE - 1);
	}

	/*
	 * Invalidate all full cache lines between 'start' and 'end'.
	 */
	while (start < end) {
		unsigned long range_end = calc_range_end(start, end);
		l2_inv_pa_range(start, range_end - CACHE_LINE_SIZE);
		start = range_end;
	}

	dsb();
}

void aurora_l2_clean_range(unsigned long start, unsigned long end)
{
    	if (!auroraL2_enable)
        	return;        
	/*
	 * If L2 is forced to WT, the L2 will always be clean and we
	 * don't need to do anything here.
	 */
	if (!l2_wt_override) {
		start &= ~(CACHE_LINE_SIZE - 1);
		end = (end + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
		while (start != end) {
			unsigned long range_end = calc_range_end(start, end);
			l2_clean_pa_range(start, range_end - CACHE_LINE_SIZE);
			start = range_end;
		}
	}

	dsb();
}

void aurora_l2_flush_range(unsigned long start, unsigned long end)
{
    	if (!auroraL2_enable)
        	return;        

	start &= ~(CACHE_LINE_SIZE - 1);
	end = (end + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
	while (start != end) {
		unsigned long range_end = calc_range_end(start, end);
		if (!l2_wt_override)
			l2_flush_pa_range(start, range_end - CACHE_LINE_SIZE);
		start = range_end;
	}

	dsb();
}
#endif /* #ifdef CONFIG_AURORA_L2_OUTER */

/*
 * Routines to disable and re-enable the D-cache and I-cache at run
 * time.  
 */
static u32 __init invalidate_and_disable_cache(void)
{
	int dummy;
	volatile u32 cr;

	cr = get_cr();
	if (cr & CR_C) {
		unsigned long flags;

		raw_local_irq_save(flags);
		flush_cache_all();
		set_cr(cr & ~CR_C);
		raw_local_irq_restore(flags);
	}
	if (cr & CR_I) {
		set_cr(cr & ~CR_I);
		__asm__ __volatile__("mcr p15, 0, %0, c7, c5, 0\n" : "=r" (dummy));
	}
	
	return ((cr & CR_C) | (cr & CR_I));
}

static void __init enable_cache(u32 mask)
{
	volatile u32 cr1;

	cr1 = get_cr();
	set_cr(cr1 | mask);
}

static void __init enable_l2(void)
{
	u32 u, mask;
#if 0
	/* Enable SMP (SMPnAMP) in Aux Control Reg */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1" : "=r" (u));
	u |= 0x40; /* Set SMPnAMP bit */
	__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1\n" : : "r" (u));
#endif
	/* Enable Broadcasring (FW) in Extra Features Reg */	
	__asm__ __volatile__("mrc p15, 1, %0, c15, c2, 0" : "=r" (u));
	u |= 0x100; /* Set the FW bit */
	__asm__ __volatile__("mcr p15, 1, %0, c15, c2, 0\n" : : "r" (u));

	u = readl(auroraL2_base+L2_CONTROL);
	if (!(u & 1)) {
		printk(KERN_INFO "Aurora L2 Cache Enabled\n");
		u |= 1;
		mask = invalidate_and_disable_cache();
		writel(u, auroraL2_base+L2_CONTROL);
		enable_cache(mask);
	}
}

/* lock_mask is a bit map. '1' means way is locked. '0' means way is unlocked */
void __init aurora_l2_lockdown(u32 cpuId, u32 lock_mask)
{
	lock_mask &= 0xFF;
	writel(lock_mask, auroraL2_base+L2_LOCKDOWN_DATA+(cpuId*8));
	writel(lock_mask, auroraL2_base+L2_LOCKDOWN_INSTR+(cpuId*8));
	writel(lock_mask, auroraL2_base+L2_LOCKDOWN_FPU);
	writel(lock_mask, auroraL2_base+L2_LOCKDOWN_IO_BRG);
}

//static u32 l2_ways = 0xffffffff; /* all ways init to L2$ ways */
static u32 l2_ways = 8;

void auroraL2_inv_all(void)
{
	writel(l2_ways, auroraL2_base+L2_INVAL_WAY_REG);
}

void auroraL2_flush_all(void)
{
    	if (!auroraL2_enable)
		return;

	writel(l2_ways, auroraL2_base + L2_FLUSH_WAY_REG);
	cache_sync();
}

/* get the L2$ ways - to enable invalidate of only the L2$ ways */
static int __init early_l2_ways(char *arg)
{
	l2_ways = memparse(arg, NULL);
	return 0;
}
early_param("l2_ways", early_l2_ways);

struct regs_entry {
	u32             reg_address;
	u32             reg_value;
};

static struct regs_entry aurora_l2_regs[] = {
	{L2_AUX_CONTROL,0},
	{L2_CONTROL, 0}
};

/* L2 Power management function */
int aurora_l2_pm_enter(void)
{
	int i;

    	if (!auroraL2_enable)
		return 0;
	for ( i = 0; i < ARRAY_SIZE(aurora_l2_regs); i++)
		aurora_l2_regs[i].reg_value = readl(auroraL2_base+ aurora_l2_regs[i].reg_address);

	return 0;
}
int aurora_l2_pm_exit(void)
{
	int i;
	u32	u;
    	if (!auroraL2_enable)
		return 0;

	u = readl(auroraL2_base+L2_CONTROL);
        if (!(u & 1)) {
		pr_debug("Aurora: Enabling L2\n");

		for ( i = 0; i < ARRAY_SIZE(aurora_l2_regs); i++)
			writel(aurora_l2_regs[i].reg_value, auroraL2_base+ aurora_l2_regs[i].reg_address);
	}
	return 0;
}

int __init aurora_l2_init(void __iomem *base)
{
	__u32 aux;

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *res;
	struct proc_dir_entry *res_file;

	res = proc_mkdir("AuroraL2", NULL);
	if (!res)
		return -ENOMEM;

    	/* Create information proc file */
	res_file = create_proc_entry("info", S_IWUSR | S_IRUGO, res);
	if (!res)
		return -ENOMEM;

	res_file->read_proc = proc_auroraL2_info_read;
	res_file->write_proc = NULL;

#ifdef CONFIG_CACHE_AURORAL2_EVENT_MONITOR_ENABLE
	/* Create counter proc file */
	res_file = create_proc_entry("counter", S_IWUSR | S_IRUGO, res);
	if (!res)
		return -ENOMEM;

	res_file->read_proc = proc_auroraL2_counter_read;
	res_file->write_proc = proc_auroraL2_counter_write;
#endif /* CONFIG_CACHE_AURORAL2_EVENT_MONITOR_ENABLE */
#endif

#ifdef CONFIG_AURORA_L2_OUTER
	outer_cache.inv_range = aurora_l2_inv_range;
	outer_cache.clean_range = aurora_l2_clean_range;
	outer_cache.flush_range = aurora_l2_flush_range;
#endif

	auroraL2_base = base;

#ifdef CONFIG_MV_AMP_ENABLE
	if(is_primary_amp())
#endif
	{

		/* 1. Write to AuroraL2 Auxiliary Control Register, 0x104
		*    Setting up Associativity, Way Size, and Latencies
		*/
		aux = readl(auroraL2_base + L2_AUX_CTRL_REG);
		aux &= ~L2ACR_REPLACEMENT_MASK;
		aux |= l2rep;

		aux &= ~L2ACR_FORCE_WRITE_POLICY_MASK;
		aux |= L2ACR_FORCE_WRITE_THRO_POLICY;
//#if defined(CONFIG_AURORA_L2_WBWT_FORCE_WB) || defined(CONFIG_MV_SUPPORT_L2_DEPOSIT)
//		aux |= L2ACR_FORCE_WRITE_BACK_POLICY;
//#elif defined(CONFIG_AURORA_L2_WBWT_FORCE_WT)
//		aux |= L2ACR_FORCE_WRITE_THRO_POLICY;
//#else /* CONFIG_AURORA_L2_WBWT_PAGE_ATTRIBUTE */
//		aux |= L2ACR_FORCE_WRITE_POLICY_DIS;
//#endif

//		aux &= ~L2ACR_FORCE_WA_MASK;
//#if defined(CONFIG_AURORA_L2_WA_FORCE_NO_ALLOCATE) || (!defined(CONFIG_AURORA_L2_ECC))
//		aux |= L2ACR_FORCE_WA_NONE;
//#elif defined(CONFIG_AURORA_L2_WA_FORCE_ALLOCATE)  || defined(CONFIG_CACHE_AURORA_L2_ERRATA_ECC_PARTIALS)
//		aux |= L2ACR_FORCE_WA_ALL;
//#else /* CONFIG_AURORA_L2_WA_PAGE_ATTRIBUTE */
//		aux |= L2ACR_FORCE_WA_REQ_ATTRIB;
//#endif

#if defined(CONFIG_AURORA_L2_ECC)
		/* Enable L2 ECC. 1bit can be corrected, 2bit will cause an excpetion */
		aux |= L2ACR_ECC_ENABLE;
#endif

		writel(aux, auroraL2_base + L2_AUX_CTRL_REG);

		l2_wt_override = ((aux & (0x3)) == 0x2 ? 1:0);
		/* 3. Secure write to AuroraL2 Invalidate by Way, 0x77c
		*/
		auroraL2_inv_all();

		/* 4. Write to the Lockdown D and Lockdown I Register 9 if required
		*/

		/* 5. Write to interrupt clear register, 0x220, to clear any residual
		*    raw interrupt set.
		*/
		writel(0x1FF, auroraL2_base + L2_INT_CAUSE_REG);

		/* 6. Enable L2 cache
		*/
		enable_l2();
	}

	auroraL2_enable = 1;

	return 0;
}
