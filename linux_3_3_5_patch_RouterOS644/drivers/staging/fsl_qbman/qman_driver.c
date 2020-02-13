/* Copyright 2008-2012 Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *	 notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *	 notice, this list of conditions and the following disclaimer in the
 *	 documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *	 names of its contributors may be used to endorse or promote products
 *	 derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "qman_private.h"

/* Global variable containing revision id (even on non-control plane systems
 * where CCSR isn't available) */
u16 qman_ip_rev;
EXPORT_SYMBOL(qman_ip_rev);

/* size of the fqd region in bytes */
#ifdef CONFIG_FSL_QMAN_FQ_LOOKUP
static u32 fqd_size = (PAGE_SIZE << CONFIG_FSL_QMAN_FQD_SZ);
#endif

/* For these variables, and the portal-initialisation logic, the
 * comments in bman_driver.c apply here so won't be repeated. */
static struct qman_portal *shared_portals[NR_CPUS];
static int num_shared_portals;
static int shared_portals_idx;

/* A SDQCR mask comprising all the available/visible pool channels */
static u32 pools_sdqcr;

#define STR_ERR_NOPROP      "No '%s' property in node %s\n"
#define STR_ERR_CELL        "'%s' is not a %d-cell range in node %s\n"
#define STR_FQID_RANGE      "fsl,fqid-range"
#define STR_POOL_CHAN_RANGE "fsl,pool-channel-range"
#define STR_CGRID_RANGE      "fsl,cgrid-range"

/* A "fsl,fqid-range" node; release the given range to the allocator */
static __init int fsl_fqid_range_init(struct device_node *node)
{
	int ret;
	const u32 *range = of_get_property(node, STR_FQID_RANGE, &ret);
	if (!range) {
		pr_err(STR_ERR_NOPROP, STR_FQID_RANGE, node->full_name);
		return -EINVAL;
	}
	if (ret != 8) {
		pr_err(STR_ERR_CELL, STR_FQID_RANGE, 2, node->full_name);
		return -EINVAL;
	}
	qman_release_fqid_range(range[0], range[1]);
	pr_info("Qman: FQID allocator includes range %d:%d\n",
		range[0], range[1]);
	return 0;
}

/* A "fsl,pool-channel-range" node; add to the SDQCR mask only */
static __init int fsl_pool_channel_range_sdqcr(struct device_node *node)
{
	int ret;
	const u32 *chanid = of_get_property(node, STR_POOL_CHAN_RANGE, &ret);
	if (!chanid) {
		pr_err(STR_ERR_NOPROP, STR_POOL_CHAN_RANGE, node->full_name);
		return -EINVAL;
	}
	if (ret != 8) {
		pr_err(STR_ERR_CELL, STR_POOL_CHAN_RANGE, 1, node->full_name);
		return -EINVAL;
	}
	for (ret = 0; ret < chanid[1]; ret++)
		pools_sdqcr |= QM_SDQCR_CHANNELS_POOL_CONV(chanid[0] + ret);
	return 0;
}

/* A "fsl,pool-channel-range" node; release the given range to the allocator */
static __init int fsl_pool_channel_range_init(struct device_node *node)
{
	int ret;
	const u32 *chanid = of_get_property(node, STR_POOL_CHAN_RANGE, &ret);
	if (!chanid) {
		pr_err(STR_ERR_NOPROP, STR_POOL_CHAN_RANGE, node->full_name);
		return -EINVAL;
	}
	if (ret != 8) {
		pr_err(STR_ERR_CELL, STR_POOL_CHAN_RANGE, 1, node->full_name);
		return -EINVAL;
	}
	qman_release_pool_range(chanid[0], chanid[1]);
	pr_info("Qman: pool channel allocator includes range %d:%d\n",
		chanid[0], chanid[1]);
	return 0;
}

/* A "fsl,cgrid-range" node; release the given range to the allocator */
static __init int fsl_cgrid_range_init(struct device_node *node)
{
	struct qman_cgr cgr;
	int ret, errors = 0;
	const u32 *range = of_get_property(node, STR_CGRID_RANGE, &ret);
	if (!range) {
		pr_err(STR_ERR_NOPROP, STR_CGRID_RANGE, node->full_name);
		return -EINVAL;
	}
	if (ret != 8) {
		pr_err(STR_ERR_CELL, STR_CGRID_RANGE, 2, node->full_name);
		return -EINVAL;
	}
	qman_release_cgrid_range(range[0], range[1]);
	pr_info("Qman: CGRID allocator includes range %d:%d\n",
		range[0], range[1]);
	for (cgr.cgrid = 0; cgr.cgrid < __CGR_NUM; cgr.cgrid++) {
		ret = qman_modify_cgr(&cgr, QMAN_CGR_FLAG_USE_INIT, NULL);
		if (ret)
			errors++;
	}
	if (errors)
		pr_err("Warning: %d error%s while initialising CGRs %d:%d\n",
			errors, (errors > 1) ? "s" : "", range[0], range[1]);
	return 0;
}

/* Parse a portal node, perform generic mapping duties and return the config. It
 * is not known at this stage for what purpose (or even if) the portal will be
 * used. */
static struct qm_portal_config * __init parse_pcfg(struct device_node *node)
{
	struct qm_portal_config *pcfg;
	const u32 *index, *channel;
	int irq, ret;
	u16 ip_rev = 0;

	pcfg = kmalloc(sizeof(*pcfg), GFP_KERNEL);
	if (!pcfg) {
		pr_err("can't allocate portal config");
		return NULL;
	}

	if (of_device_is_compatible(node, "fsl,qman-portal-1.0"))
		ip_rev = QMAN_REV10;
	else if (of_device_is_compatible(node, "fsl,qman-portal-1.1"))
		ip_rev = QMAN_REV11;
	else if	(of_device_is_compatible(node, "fsl,qman-portal-1.2"))
		ip_rev = QMAN_REV12;
	else if (of_device_is_compatible(node, "fsl,qman-portal-2.0"))
		ip_rev = QMAN_REV20;

	if (!qman_ip_rev) {
		if (ip_rev)
			qman_ip_rev = ip_rev;
		else {
			pr_warning("unknown Qman version, default to rev1.1\n");
			qman_ip_rev = QMAN_REV11;
		}
	} else if (ip_rev && (qman_ip_rev != ip_rev))
		pr_warning("Revision=0x%04x, but portal '%s' has 0x%04x\n",
			qman_ip_rev, node->full_name, ip_rev);

	ret = of_address_to_resource(node, DPA_PORTAL_CE,
				&pcfg->addr_phys[DPA_PORTAL_CE]);
	if (ret) {
		pr_err("Can't get %s property '%s'\n", node->full_name,
			"reg::CE");
		goto err;
	}
	ret = of_address_to_resource(node, DPA_PORTAL_CI,
				&pcfg->addr_phys[DPA_PORTAL_CI]);
	if (ret) {
		pr_err("Can't get %s property '%s'\n", node->full_name,
			"reg::CI");
		goto err;
	}
	index = of_get_property(node, "cell-index", &ret);
	if (!index || (ret != 4)) {
		pr_err("Can't get %s property '%s'\n", node->full_name,
			"cell-index");
		goto err;
	}
	channel = of_get_property(node, "fsl,qman-channel-id", &ret);
	if (!channel || (ret != 4)) {
		pr_err("Can't get %s property '%s'\n", node->full_name,
			"fsl,qman-channel-id");
		goto err;
	}
	if (*channel != (*index + qm_channel_swportal0))
		pr_err("Warning: node %s has mismatched %s and %s\n",
			node->full_name, "cell-index", "fsl,qman-channel-id");
	pcfg->public_cfg.channel = *channel;
	pcfg->public_cfg.cpu = -1;
	irq = irq_of_parse_and_map(node, 0);
	if (irq == NO_IRQ) {
		pr_err("Can't get %s property '%s'\n", node->full_name,
			"interrupts");
		goto err;
	}
	pcfg->public_cfg.irq = irq;
	pcfg->public_cfg.index = *index;
	pcfg->node = node;
#ifdef CONFIG_FSL_QMAN_CONFIG
	/* We need the same LIODN offset for all portals */
	qman_liodn_fixup(pcfg->public_cfg.channel);
#endif

	pcfg->addr_virt[DPA_PORTAL_CE] = ioremap_prot(
				pcfg->addr_phys[DPA_PORTAL_CE].start,
				resource_size(&pcfg->addr_phys[DPA_PORTAL_CE]),
				0);
	pcfg->addr_virt[DPA_PORTAL_CI] = ioremap_prot(
				pcfg->addr_phys[DPA_PORTAL_CI].start,
				resource_size(&pcfg->addr_phys[DPA_PORTAL_CI]),
				_PAGE_GUARDED | _PAGE_NO_CACHE);

	return pcfg;
err:
	kfree(pcfg);
	return NULL;
}

/* Destroy a previously-parsed portal config. */
static void destroy_pcfg(struct qm_portal_config *pcfg)
{
	iounmap(pcfg->addr_virt[DPA_PORTAL_CI]);
	iounmap(pcfg->addr_virt[DPA_PORTAL_CE]);
	kfree(pcfg);
}

static struct qm_portal_config *get_pcfg(struct list_head *list)
{
	struct qm_portal_config *pcfg;
	if (list_empty(list))
		return NULL;
	pcfg = list_entry(list->prev, struct qm_portal_config, list);
	list_del(&pcfg->list);
	return pcfg;
}

#ifdef CONFIG_FSL_PAMU
static void portal_set_liodns(const struct qm_portal_config *pcfg, int cpu)
{
	unsigned int index = 0;
	unsigned int liodn_cnt = pamu_get_liodn_count(pcfg->node);
	while (index < liodn_cnt) {
		int ret = pamu_set_stash_dest(pcfg->node, index++, cpu, 1);
		if (ret < 0)
			pr_warning("Failed to set QMan stashing LIODN\n");
	}
}
#else
#define portal_set_liodns(pcfg, cpu) do { } while (0)
#endif

static void portal_set_cpu(const struct qm_portal_config *pcfg, int cpu)
{
	portal_set_liodns(pcfg, cpu);
#ifdef CONFIG_FSL_QMAN_CONFIG
	if (qman_set_sdest(pcfg->public_cfg.channel, cpu))
#endif
		pr_warning("Failed to set QMan portal's stash request queue\n");
}

/* UIO handling callbacks */
#define QMAN_UIO_PREAMBLE() \
	const struct qm_portal_config *pcfg = \
		container_of(__p, struct qm_portal_config, list)
static int qman_uio_cb_init(const struct list_head *__p, struct uio_info *info)
{
	QMAN_UIO_PREAMBLE();
	/* big enough for "qman-uio-xx" */
	char *name = kzalloc(16, GFP_KERNEL);
	if (!name)
		return -ENOMEM;
	sprintf(name, "qman-uio-%x", pcfg->public_cfg.index);
	info->name = name;
	info->mem[DPA_PORTAL_CE].name = "cena";
	info->mem[DPA_PORTAL_CE].addr = pcfg->addr_phys[DPA_PORTAL_CE].start;
	info->mem[DPA_PORTAL_CE].size =
		resource_size(&pcfg->addr_phys[DPA_PORTAL_CE]);
	info->mem[DPA_PORTAL_CE].memtype = UIO_MEM_PHYS;
	info->mem[DPA_PORTAL_CI].name = "cinh";
	info->mem[DPA_PORTAL_CI].addr = pcfg->addr_phys[DPA_PORTAL_CI].start;
	info->mem[DPA_PORTAL_CI].size =
		resource_size(&pcfg->addr_phys[DPA_PORTAL_CI]);
	info->mem[DPA_PORTAL_CI].memtype = UIO_MEM_PHYS;
	info->irq = pcfg->public_cfg.irq;
	return 0;
}
static void qman_uio_cb_destroy(const struct list_head *__p,
				struct uio_info *info)
{
	QMAN_UIO_PREAMBLE();
	kfree(info->name);
	/* We own this struct but had passed it to the dpa_uio layer as a const
	 * so that we don't accidentally meddle with it in the dpa_uio code.
	 * Here it's passed back to us for final clean it up, so de-constify. */
	destroy_pcfg((struct qm_portal_config *)pcfg);
}
static int qman_uio_cb_open(const struct list_head *__p)
{
	QMAN_UIO_PREAMBLE();
	/* Bind stashing LIODNs to the CPU we are currently executing on, and
	 * set the portal to use the stashing request queue corresonding to the
	 * cpu as well. The user-space driver assumption is that the pthread has
	 * to already be affine to one cpu only before opening a portal. If that
	 * check is circumvented, the only risk is a performance degradation -
	 * stashing will go to whatever cpu they happened to be running on when
	 * opening the device file, and if that isn't the cpu they subsequently
	 * bind to and do their polling on, tough. */
	portal_set_cpu(pcfg, smp_processor_id());
	return 0;
}
static void qman_uio_cb_interrupt(const struct list_head *__p)
{
	QMAN_UIO_PREAMBLE();
	/* This is the only manipulation of a portal register that isn't in the
	 * regular kernel portal driver (_high.c/_low.h). It is also the only
	 * time the kernel touches a register on a portal that is otherwise
	 * being driven by a user-space driver. So rather than messing up
	 * encapsulation for one trivial call, I am hard-coding the offset to
	 * the inhibit register and writing it directly from here. */
	out_be32(pcfg->addr_virt[DPA_PORTAL_CI] + 0xe0c, ~(u32)0);
}
static const struct dpa_uio_vtable qman_uio = {
	.init_uio = qman_uio_cb_init,
	.destroy = qman_uio_cb_destroy,
	.on_open = qman_uio_cb_open,
	.on_interrupt = qman_uio_cb_interrupt
};

static struct qman_portal *init_pcfg(struct qm_portal_config *pcfg)
{
	struct qman_portal *p;
	struct cpumask oldmask = *tsk_cpus_allowed(current);

	portal_set_cpu(pcfg, pcfg->public_cfg.cpu);
	set_cpus_allowed_ptr(current, get_cpu_mask(pcfg->public_cfg.cpu));
	p = qman_create_affine_portal(pcfg, NULL);
	if (p) {
		u32 irq_sources = 0;
		/* Determine what should be interrupt-vs-poll driven */
#ifdef CONFIG_FSL_DPA_PIRQ_SLOW
		irq_sources |= QM_PIRQ_EQCI | QM_PIRQ_EQRI | QM_PIRQ_MRI |
			       QM_PIRQ_CSCI;
#endif
#ifdef CONFIG_FSL_DPA_PIRQ_FAST
		irq_sources |= QM_PIRQ_DQRI;
#endif
		qman_irqsource_add(irq_sources);
		pr_info("Qman portal %sinitialised, cpu %d\n",
			pcfg->public_cfg.is_shared ? "(shared) " : "",
			pcfg->public_cfg.cpu);
	} else
		pr_crit("Qman portal failure on cpu %d\n",
			pcfg->public_cfg.cpu);
	set_cpus_allowed_ptr(current, &oldmask);
	return p;
}

static void init_slave(int cpu)
{
	struct qman_portal *p;
	struct cpumask oldmask = *tsk_cpus_allowed(current);
	set_cpus_allowed_ptr(current, get_cpu_mask(cpu));
	p = qman_create_affine_slave(shared_portals[shared_portals_idx++]);
	if (!p)
		pr_err("Qman slave portal failure on cpu %d\n", cpu);
	else
		pr_info("Qman portal %sinitialised, cpu %d\n", "(slave) ", cpu);
	set_cpus_allowed_ptr(current, &oldmask);
	if (shared_portals_idx >= num_shared_portals)
		shared_portals_idx = 0;
}

static struct cpumask want_unshared __initdata;
static struct cpumask want_shared __initdata;

static int __init parse_qportals(char *str)
{
	return parse_portals_bootarg(str, &want_shared, &want_unshared,
				     "qportals");
}
__setup("qportals=", parse_qportals);

static __init int qman_init(void)
{
	struct cpumask slave_cpus;
	struct cpumask unshared_cpus = *cpu_none_mask;
	struct cpumask shared_cpus = *cpu_none_mask;
	LIST_HEAD(unused_pcfgs);
	LIST_HEAD(unshared_pcfgs);
	LIST_HEAD(shared_pcfgs);
	struct device_node *dn;
	struct qm_portal_config *pcfg;
	struct qman_portal *p;
	int cpu, ret;

	/* Initialise the Qman (CCSR) device */
	for_each_compatible_node(dn, NULL, "fsl,qman") {
		if (!qman_init_ccsr(dn))
			pr_info("Qman err interrupt handler present\n");
		else
			pr_err("Qman CCSR setup failed\n");
	}
#ifdef CONFIG_FSL_QMAN_FQ_LOOKUP
	/* Setup lookup table for FQ demux */
	ret = qman_setup_fq_lookup_table(fqd_size/64);
	if (ret)
		return ret;
#endif
	/* Parse pool channels into the SDQCR mask. (Must happen before portals
	 * are initialised.) */
	for_each_compatible_node(dn, NULL, "fsl,pool-channel-range") {
		ret = fsl_pool_channel_range_sdqcr(dn);
		if (ret)
			return ret;
	}
	/* Initialise portals. See bman_driver.c for comments */
	for_each_compatible_node(dn, NULL, "fsl,qman-portal") {
		if (!of_device_is_available(dn))
			continue;
		pcfg = parse_pcfg(dn);
		if (pcfg) {
			pcfg->public_cfg.pools = pools_sdqcr;
			list_add_tail(&pcfg->list, &unused_pcfgs);
		}
	}
	for_each_cpu(cpu, &want_shared) {
		pcfg = get_pcfg(&unused_pcfgs);
		if (!pcfg)
			break;
		pcfg->public_cfg.cpu = cpu;
		list_add_tail(&pcfg->list, &shared_pcfgs);
		cpumask_set_cpu(cpu, &shared_cpus);
	}
	for_each_cpu(cpu, &want_unshared) {
		if (cpumask_test_cpu(cpu, &shared_cpus))
			continue;
		pcfg = get_pcfg(&unused_pcfgs);
		if (!pcfg)
			break;
		pcfg->public_cfg.cpu = cpu;
		list_add_tail(&pcfg->list, &unshared_pcfgs);
		cpumask_set_cpu(cpu, &unshared_cpus);
	}
	if (list_empty(&shared_pcfgs) && list_empty(&unshared_pcfgs)) {
		for_each_online_cpu(cpu) {
			pcfg = get_pcfg(&unused_pcfgs);
			if (!pcfg)
				break;
			pcfg->public_cfg.cpu = cpu;
			list_add_tail(&pcfg->list, &unshared_pcfgs);
			cpumask_set_cpu(cpu, &unshared_cpus);
		}
	}
	cpumask_andnot(&slave_cpus, cpu_online_mask, &shared_cpus);
	cpumask_andnot(&slave_cpus, &slave_cpus, &unshared_cpus);
	if (cpumask_empty(&slave_cpus)) {
		if (!list_empty(&shared_pcfgs)) {
			cpumask_or(&unshared_cpus, &unshared_cpus,
				   &shared_cpus);
			cpumask_clear(&shared_cpus);
			list_splice_tail(&shared_pcfgs, &unshared_pcfgs);
			INIT_LIST_HEAD(&shared_pcfgs);
		}
	} else {
		if (list_empty(&shared_pcfgs)) {
			pcfg = get_pcfg(&unshared_pcfgs);
			if (!pcfg) {
				pr_crit("No QMan portals available!\n");
				return 0;
			}
			cpumask_clear_cpu(pcfg->public_cfg.cpu, &unshared_cpus);
			cpumask_set_cpu(pcfg->public_cfg.cpu, &shared_cpus);
			list_add_tail(&pcfg->list, &shared_pcfgs);
		}
	}
	list_for_each_entry(pcfg, &unshared_pcfgs, list) {
		pcfg->public_cfg.is_shared = 0;
		p = init_pcfg(pcfg);
	}
	list_for_each_entry(pcfg, &shared_pcfgs, list) {
		pcfg->public_cfg.is_shared = 1;
		p = init_pcfg(pcfg);
		if (p)
			shared_portals[num_shared_portals++] = p;
	}
	if (!cpumask_empty(&slave_cpus))
		for_each_cpu(cpu, &slave_cpus)
			init_slave(cpu);
	pr_info("Qman portals initialised\n");
#ifdef CONFIG_FSL_DPA_UIO
	/* Export any left over portals as UIO devices */
	do {
		pcfg = get_pcfg(&unused_pcfgs);
		if (!pcfg)
			break;
		ret = dpa_uio_register(&pcfg->list, &qman_uio);
		if (ret) {
			pr_err("Failure registering QMan UIO portal\n");
			destroy_pcfg(pcfg);
		}
	} while (1);
#endif
	/* Initialise FQID allocation ranges */
	for_each_compatible_node(dn, NULL, "fsl,fqid-range") {
		ret = fsl_fqid_range_init(dn);
		if (ret)
			return ret;
	}
	/* Initialise CGRID allocation ranges */
	for_each_compatible_node(dn, NULL, "fsl,cgrid-range") {
		ret = fsl_cgrid_range_init(dn);
		if (ret)
			return ret;
	}
	/* Parse pool channels into the allocator. (Must happen after portals
	 * are initialised.) */
	for_each_compatible_node(dn, NULL, "fsl,pool-channel-range") {
		ret = fsl_pool_channel_range_init(dn);
		if (ret)
			return ret;
	}
	return 0;
}
subsys_initcall(qman_init);
