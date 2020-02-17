#ifndef _ASM_POWERPC_RB_AUX_H
#define _ASM_POWERPC_RB_AUX_H

#include <linux/seq_file.h>
#include <linux/init.h>

extern void __init rb_pic_init(void);
extern void __init rb_init_pci(void);
extern void rb_show_cpuinfo(struct seq_file *);
extern void rb_restart(char *cmd);
extern void rb_idle(void);
extern void change_latch(unsigned char set, unsigned char clear);
extern unsigned get_gpio_def(const char *name);

void add_second_serial_of_node(void);
void add_crypto_of_node(unsigned irq);

#endif
