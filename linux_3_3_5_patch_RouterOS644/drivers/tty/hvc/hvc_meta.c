#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/slab.h>
#include <asm/vm.h>
#include <asm/irq.h>
#include "hvc_console.h"

extern int vm_create_queue(unsigned id, unsigned irq,
			   unsigned tx, unsigned rx);
extern int vm_release_queue(unsigned id);

#define BUF_SIZE	4096

static volatile struct vdma_descr tx_descr;
static volatile struct vdma_descr rx_descr;

static unsigned rx_offset;
static DEFINE_SPINLOCK(lock);

static int put_chars(u32 vtermno, const char *buf, int count)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&lock, flags);

#ifdef __powerpc__
	for (i = 0; i < 2000000; ++i) {
#else
	for (i = 0; i < 2; ++i) {
#endif
		unsigned size = xchg(&tx_descr.size, 0);

		if (!(size & DONE)) {
			count = min(count, BUF_SIZE);
			memcpy((char *) tx_descr.addr, buf, count);
			tx_descr.size = count | DONE;

			spin_unlock_irqrestore(&lock, flags);
			return count;
		}

		if (size == (BUF_SIZE | DONE)) {
			if (i == 0) {
				tx_descr.size = size;
				hc_yield();
				continue;
			} else {
				unsigned drop = BUF_SIZE / 4;
				size = BUF_SIZE - drop;
				memcpy((char *) tx_descr.addr,
				       (char *) tx_descr.addr + drop,
				       size);
			}
		}

		size &= ~DONE;
		count = min(BUF_SIZE - (int) size, count);
		memcpy((char *) tx_descr.addr + size, buf, count);
		tx_descr.size = (size + count) | DONE;

		spin_unlock_irqrestore(&lock, flags);
		return count;
	}

	spin_unlock_irqrestore(&lock, flags);
	return 0;
}

static int get_chars(u32 vtermno, char *buf, int count)
{
	unsigned long flags;
	unsigned size;

	spin_lock_irqsave(&lock, flags);

	if (!(rx_descr.size & DONE)) {
		spin_unlock_irqrestore(&lock, flags);
		return -EAGAIN;
	}
	
	size = (rx_descr.size & ~DONE) - rx_offset;
	count = min(count, (int) size);

	memcpy(buf, (char *) rx_descr.addr + rx_offset, count);

	if (count == size) {
		rx_descr.size = BUF_SIZE;
		rx_offset = 0;
	} else {
		rx_offset += count;
	}

	spin_unlock_irqrestore(&lock, flags);
	return count;
}

static struct hv_ops cons = {
	.put_chars = put_chars,
	.get_chars = get_chars,
	.notifier_add = notifier_add_irq,
	.notifier_del = notifier_del_irq,
};

static int __init cons_init(void)
{
	if (vm_running() != 0)
		return 0;

	rx_descr.addr = (unsigned) kmalloc(BUF_SIZE, GFP_KERNEL);
	rx_descr.size = BUF_SIZE;
	rx_descr.next = (unsigned) &rx_descr;
	
	tx_descr.addr = (unsigned) kmalloc(BUF_SIZE, GFP_KERNEL);
	tx_descr.size = 0;
	tx_descr.next = (unsigned) &tx_descr;

	vm_create_queue(1, 1,
			(unsigned) &tx_descr, (unsigned) &rx_descr);

	return hvc_instantiate(0, 0, &cons);
}
console_initcall(cons_init);

int vm_init(void)
{
	if (vm_running() == 0) 
		hvc_alloc(0, get_virq_nr(1), &cons, 256);
	return 0;
}
module_init(vm_init);
