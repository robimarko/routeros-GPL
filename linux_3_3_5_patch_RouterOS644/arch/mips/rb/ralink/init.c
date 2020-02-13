#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/delay.h>
#include <linux/initrd.h>
#include <linux/mm.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/mach-ralink/generic.h>
#include <asm/mach-ralink/surfboard.h>
#include <asm/mach-ralink/surfboardint.h>
#include <asm/mach-ralink/rt_mmap.h>
#include <asm/mach-ralink/serial_rt2880.h>
#if defined (CONFIG_IRQ_GIC)
#include <asm/gcmpregs.h>
#endif
#include <asm/mips-boards/launch.h>
#include <asm/reboot.h>
#include <asm/cpu.h>
#include <asm/irq.h>
#include <asm/time.h>
#include <asm/traps.h>
#include <asm/sections.h>


static struct uart_port serial_req;

#define LPS_PREC 8
/*
 *  Re-calibration lpj(loop-per-jiffy).
 *  (derived from kernel/calibrate.c)
 */
static int udelay_recal(void) {
	unsigned int i, lpj = 0;
	unsigned long ticks, loopbit;
	int lps_precision = LPS_PREC;

	lpj = (1<<12);

	while ((lpj <<= 1) != 0) {
		/* wait for "start of" clock tick */
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */;

			/* Go .. */
		ticks = jiffies;
		__delay(lpj);
		ticks = jiffies - ticks;
		if (ticks)
			break;
	}

	/*
	 * Do a binary approximation to get lpj set to
	 * equal one clock (up to lps_precision bits)
	 */
	lpj >>= 1;
	loopbit = lpj;
	while (lps_precision-- && (loopbit >>= 1)) {
		lpj |= loopbit;
		ticks = jiffies;
		while (ticks == jiffies)
				/* nothing */;
		ticks = jiffies;
		__delay(lpj);
		if (jiffies != ticks)   /* longer than 1 tick */
			lpj &= ~loopbit;
	}
	printk("%d CPUs re-calibrate udelay(lpj = %d)\n", NR_CPUS, lpj);

	for(i=0; i< NR_CPUS; i++)
		cpu_data[i].udelay_val = lpj;

#if defined (CONFIG_RALINK_CPUSLEEP) && defined (CONFIG_RALINK_MT7621)
	lpj = (*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x10)));
        lpj &= ~(0xF << 8);
        lpj |= (0xA << 8);
        (*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x10))) = lpj;
#endif
	return 0;
}
device_initcall(udelay_recal);

static void mips_machine_restart(char *command) {
	*(volatile unsigned int*)(SOFTRES_REG) = GORESET;
	*(volatile unsigned int*)(SOFTRES_REG) = 0;
}

static void mips_machine_halt(void) {
	*(volatile unsigned int*)(SOFTRES_REG) = (0x1)<<26; // PCIERST
	mdelay(10);
	*(volatile unsigned int*)(SOFTRES_REG) = GORESET;
	*(volatile unsigned int*)(SOFTRES_REG) = 0;
}

static void mips_machine_power_off(void) {
	*(volatile unsigned int*)(POWER_DIR_REG) = POWER_DIR_OUTPUT;
	*(volatile unsigned int*)(POWER_POL_REG) = 0;
	*(volatile unsigned int*)(POWEROFF_REG) = POWEROFF;
}

void __init mmips_setup(void)
{
	iomem_resource.start = 0;
	iomem_resource.end= ~0;
	ioport_resource.start= 0;
	ioport_resource.end = ~0;

#ifdef CONFIG_MIPS_IOCU
        int supported = 0;
        if (gcmp_niocu() != 0) {
                /* Nothing special needs to be done to enable coherency */
                printk("CMP IOCU detected\n");
                supported = 1;
        }
        hw_coherentio = supported;
#endif

	_machine_restart = mips_machine_restart;
	_machine_halt = mips_machine_halt;
	pm_power_off = mips_machine_power_off;
}

int amon_cpu_avail(int cpu) {
	struct cpulaunch *launch = (struct cpulaunch *)CKSEG0ADDR(CPULAUNCH);

	if (cpu < 0 || cpu >= NCPULAUNCH) {
		printk("avail: cpu%d is out of range\n", cpu);
		return 0;
	}

	launch += cpu;
	if (!(launch->flags & LAUNCH_FREADY)) {
		printk("avail: cpu%d is not ready\n", cpu);
		return 0;
	}
	if (launch->flags & (LAUNCH_FGO|LAUNCH_FGONE)) {
		printk("avail: too late.. cpu%d is already gone\n", cpu);
		return 0;
	}

	return 1;
}

void amon_cpu_start(int cpu,
		    unsigned long pc, unsigned long sp,
		    unsigned long gp, unsigned long a0) {
	volatile struct cpulaunch *launch =
		(struct cpulaunch  *)CKSEG0ADDR(CPULAUNCH);

	if (!amon_cpu_avail(cpu)) {
		return;
	}
	if (cpu == smp_processor_id()) {
		printk("launch: I am cpu%d!\n", cpu);
		return;
	}
	launch += cpu;

	printk("launch: starting cpu%d\n", cpu);

	launch->pc = pc;
	launch->gp = gp;
	launch->sp = sp;
	launch->a0 = a0;

	smp_wmb();		/* Target must see parameters before go */
	launch->flags |= LAUNCH_FGO;
	smp_wmb();		/* Target must see go before we poll  */
	while ((launch->flags & LAUNCH_FGONE) == 0)
		;
	smp_rmb();		/* Target will be updating flags soon */
	printk("launch: cpu%d gone!\n", cpu);
}

#define BOARD_TAG  "board="

__init void prom_init_ralink(void) {
	unsigned int clock_divisor = (50000000 / SURFBOARD_BAUD_DIV / 115200);

	write_c0_wired(0);

        //fix at 57600 8 n 1 n
        IER(RALINK_SYSCTL_BASE + 0xC00) = 0;
        FCR(RALINK_SYSCTL_BASE + 0xC00) = 0;
        LCR(RALINK_SYSCTL_BASE + 0xC00) = (UART_LCR_WLEN8 | UART_LCR_DLAB);
        DLL(RALINK_SYSCTL_BASE + 0xC00) = clock_divisor & 0xff;
        DLM(RALINK_SYSCTL_BASE + 0xC00) = clock_divisor >> 8;
        LCR(RALINK_SYSCTL_BASE + 0xC00) = UART_LCR_WLEN8;

        IER(RALINK_SYSCTL_BASE + 0xD00) = 0;
        FCR(RALINK_SYSCTL_BASE + 0xD00) = 0;
        LCR(RALINK_SYSCTL_BASE + 0xD00) = (UART_LCR_WLEN8 | UART_LCR_DLAB);
        DLL(RALINK_SYSCTL_BASE + 0xD00) = clock_divisor & 0xff;
        DLM(RALINK_SYSCTL_BASE + 0xD00) = clock_divisor >> 8;
        LCR(RALINK_SYSCTL_BASE + 0xD00) = UART_LCR_WLEN8;

	/*
	 * baud rate = system clock freq / (CLKDIV * 16)
	 * CLKDIV=system clock freq/16/baud rate
	 */
	memset(&serial_req, 0, sizeof(struct uart_port));

	serial_req.type       = PORT_16550A;
	serial_req.line       = 0;
	serial_req.irq        = SURFBOARDINT_UART1;
	serial_req.flags      = UPF_FIXED_TYPE;
	serial_req.uartclk    = 50000000;
	serial_req.iotype     = UPIO_MEM32;
	serial_req.regshift   = 2;
	serial_req.mapbase    = RALINK_UART_LITE_BASE;
	serial_req.membase    = ioremap_nocache(RALINK_UART_LITE_BASE, PAGE_SIZE);

	early_serial_setup(&serial_req);

	if (gcmp_probe(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ)){
		if (!register_cmp_smp_ops())
			return;
        }                                                        
	if (!register_vsmp_smp_ops())
		return;
}
