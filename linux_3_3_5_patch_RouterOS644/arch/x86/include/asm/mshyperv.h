#ifndef _ASM_X86_MSHYPER_H
#define _ASM_X86_MSHYPER_H

#include <linux/types.h>
#include <linux/interrupt.h>
#include <asm/hyperv.h>

struct ms_hyperv_info {
	u32 features;
	u32 hints;
};

extern struct ms_hyperv_info ms_hyperv;

void hyperv_callback_vector(void);
void hyperv_vector_handler(struct pt_regs *regs);
void hv_setup_vmbus_irq(void (*handler)(void));
void hv_remove_vmbus_irq(void);
void hv_setup_kexec_handler(void (*handler)(void));
void hv_remove_kexec_handler(void);
void hv_setup_crash_handler(void (*handler)(struct pt_regs *regs));
void hv_remove_crash_handler(void);

#endif