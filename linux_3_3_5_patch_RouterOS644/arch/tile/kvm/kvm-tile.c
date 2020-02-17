/*
 * Copyright 2011 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/kvm.h>
#include <linux/kvm_host.h>
#include <linux/kvm_para.h>
#include <linux/kvm_types.h>
#include <linux/module.h>
#include <linux/types.h>


#include <asm/kvm_host.h>

struct kvm_stats_debugfs_item debugfs_entries[] = {
	{ NULL }
};

int kvm_arch_prepare_memory_region(struct kvm *kvm,
				   struct kvm_memory_slot *memslot,
				   struct kvm_memory_slot old,
				   struct kvm_userspace_memory_region *mem,
				   int user_alloc)
{
	printk("kvm_arch_prepare_memory_region\n");

	return 0;
}

void kvm_arch_commit_memory_region(struct kvm *kvm,
				   struct kvm_userspace_memory_region *mem,
				   struct kvm_memory_slot old,
				   int user_alloc)
{
	printk("kvm_arch_commit_memory_region\n");
}

void kvm_arch_flush_shadow(struct kvm *kvm)
{
	printk("kvm_arch_flush_shadow\n");
}

gfn_t unalias_gfn(struct kvm *kvm, gfn_t gfn)
{
	printk("unalias_gfn\n");
	return 0;
}

long kvm_arch_dev_ioctl(struct file *filp,
			unsigned int ioctl, unsigned long arg)
{
	printk("kvm_arch_dev_ioctl\n");
	return 0;
}

long kvm_arch_vcpu_ioctl(struct file *filp,
			 unsigned int ioctl, unsigned long arg)
{
	printk("kvm_arch_vcpu_ioctl\n");
	return 0;
}

int kvm_dev_ioctl_check_extension(long ext)
{
	printk("kvm_dev_ioctl_check_extension\n");
	return 0;
}

int kvm_vm_ioctl_get_dirty_log(struct kvm *kvm,
			       struct kvm_dirty_log *log)
{
	printk("kvm_vm_ioctl_get_dirty_log\n");
	return 0;
}

long kvm_arch_vm_ioctl(struct file *filp,
		       unsigned int ioctl, unsigned long arg)
{
	printk("kvm_arch_vm_ioctl\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_get_fpu(struct kvm_vcpu *vcpu, struct kvm_fpu *fpu)
{
	printk("kvm_arch_vcpu_ioctl_get_fpu\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_set_fpu(struct kvm_vcpu *vcpu, struct kvm_fpu *fpu)
{
	printk("kvm_arch_vcpu_ioctl_set_fpu\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_translate(struct kvm_vcpu *vcpu,
				  struct kvm_translation *tr)
{
	printk("kvm_arch_vcpu_ioctl_translate\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_get_regs(struct kvm_vcpu *vcpu, struct kvm_regs *regs)
{
	printk("kvm_arch_vcpu_ioctl_get_regs\n");

	memcpy(regs->regs, vcpu->arch.regs, sizeof (vcpu->arch.regs));
	regs->tp = vcpu->arch.tp;
	regs->sp = vcpu->arch.sp;
	regs->lr = vcpu->arch.lr;
	regs->pc = vcpu->arch.pc;
	regs->ex1 = vcpu->arch.ex1;

	return 0;
}

int kvm_arch_vcpu_ioctl_set_regs(struct kvm_vcpu *vcpu, struct kvm_regs *regs)
{
	printk("kvm_arch_vcpu_ioctl_set_regs\n");

	memcpy(vcpu->arch.regs, regs->regs, sizeof (vcpu->arch.regs));
	vcpu->arch.tp = regs->tp;
	vcpu->arch.sp = regs->sp;
	vcpu->arch.lr = regs->lr;
	vcpu->arch.pc = regs->pc;
	vcpu->arch.ex1 = regs->ex1;

	return 0;
}

int kvm_arch_vcpu_ioctl_get_sregs(struct kvm_vcpu *vcpu,
				  struct kvm_sregs *sregs)
{
	printk("kvm_arch_vcpu_ioctl_get_sregs\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_set_sregs(struct kvm_vcpu *vcpu,
				  struct kvm_sregs *sregs)
{
	printk("kvm_arch_vcpu_ioctl_set_sregs\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_get_mpstate(struct kvm_vcpu *vcpu,
				    struct kvm_mp_state *mp_state)
{
	printk("kvm_arch_vcpu_ioctl_get_mpstate\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_set_mpstate(struct kvm_vcpu *vcpu,
				    struct kvm_mp_state *mp_state)
{
	printk("kvm_arch_vcpu_ioctl_set_mpstate\n");
	return 0;
}

int kvm_arch_vcpu_ioctl_set_guest_debug(struct kvm_vcpu *vcpu,
					struct kvm_guest_debug *dbg)
{
	printk("kvm_arch_vcpu_ioctl_set_guest_debug\n");
	return 0;
}


int kvm_arch_vcpu_ioctl_run(struct kvm_vcpu *vcpu, struct kvm_run *kvm_run)
{
	int r;
	sigset_t sigsaved;
printk("kvm_arch_vcpu_ioctl_run\n");

	vcpu_load(vcpu);

	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &vcpu->sigset, &sigsaved);

	local_irq_disable();
	kvm_guest_enter();
#if 1
	kvm_run->exit_reason = KVM_EXIT_SHUTDOWN;
	r = 0;
#else
	r = __vcpu_run(run, vcpu);
#endif
	kvm_guest_exit();
	local_irq_enable();

	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &sigsaved, NULL);

	vcpu_put(vcpu);

	return r;
}

int kvm_arch_init(void *opaque)
{
	printk("kvm_arch_init\n");
	return 0;
}

void kvm_arch_exit(void)
{
	printk("kvm_arch_exit\n");
}

int kvm_arch_vcpu_init(struct kvm_vcpu *vcpu)
{
	printk("kvm_arch_vcpu_init\n");
	return 0;
}

void kvm_arch_vcpu_uninit(struct kvm_vcpu *vcpu)
{
	printk("kvm_arch_vcpu_uninit\n");
}

void kvm_arch_vcpu_load(struct kvm_vcpu *vcpu, int cpu)
{
	printk("kvm_arch_vcpu_load\n");
}

void kvm_arch_vcpu_put(struct kvm_vcpu *vcpu)
{
	printk("kvm_arch_vcpu_put\n");
}

struct kvm_vcpu *kvm_arch_vcpu_create(struct kvm *kvm, unsigned int id)
{
	// FIXME some archs set up a cache for these structs?
	struct kvm_vcpu *vcpu = kzalloc(sizeof(struct kvm_vcpu), GFP_KERNEL);
	int rc;

printk("kvm_arch_vcpu_create\n");

	if (!vcpu)
		return ERR_PTR(-ENOMEM);

	rc = kvm_vcpu_init(vcpu, kvm, id);
	if (rc) {
		kfree(vcpu);
		return ERR_PTR(rc);
	}

	return vcpu;
}

int kvm_arch_vcpu_setup(struct kvm_vcpu *vcpu)
{
	printk("kvm_arch_vcpu_setup\n");
	return 0;
}

void kvm_arch_vcpu_destroy(struct kvm_vcpu *vcpu)
{
	printk("kvm_arch_vcpu_destroy\n");

	kfree(vcpu);
}

int kvm_arch_hardware_enable(void *garbage)
{
	return 0;
}

void kvm_arch_hardware_disable(void *garbage)
{
}

int kvm_arch_hardware_setup(void)
{
	printk("kvm_arch_hardware_setup\n");
	return 0;
}

void kvm_arch_hardware_unsetup(void)
{
	printk("kvm_arch_hardware_unsetup\n");
}

void kvm_arch_check_processor_compat(void *rtn)
{
	printk("kvm_arch_check_processor_compat\n");
}

int kvm_arch_vcpu_runnable(struct kvm_vcpu *vcpu)
{
	printk("kvm_arch_vcpu_runnable\n");
	return 0;
}

struct kvm *kvm_arch_create_vm(void)
{
	struct kvm *kvm;
printk("kvm_arch_create_vm\n");

	kvm = kzalloc(sizeof(struct kvm), GFP_KERNEL);
	if (!kvm)
		return ERR_PTR(-ENOMEM);

	return kvm;
}

void kvm_arch_destroy_vm(struct kvm *kvm)
{
printk("kvm_arch_destroy_vm\n");
	kfree(kvm);
}

void kvm_arch_sync_events(struct kvm *kvm)
{
	printk("kvm_arch_sync_events\n");
}

int kvm_cpu_has_pending_timer(struct kvm_vcpu *vcpu)
{
	printk("kvm_cpu_has_pending_timer\n");
	return 0;
}

static int __init kvm_tile_init(void)
{
	return kvm_init(NULL, sizeof(struct kvm_vcpu), THIS_MODULE);
}

static void __exit kvm_tile_exit(void)
{
	kvm_exit();
}

module_init(kvm_tile_init);
module_exit(kvm_tile_exit);
