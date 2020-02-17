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
 *
 * memprof.c - driver for controlling HV memory profiling.
 *
 * The hypervisor's memory controller profiling infrastructure allows
 * the programmer to find out what fraction of the available memory
 * bandwidth is being consumed at each memory controller.  The
 * profiler provides start, stop, and clear operations to allows
 * profiling over a specific time window, as well as an interface for
 * reading the most recent profile values.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <asm/memprof.h>
#include <hv/drv_memprof_intf.h>

static char hv_filename[] = "memprof";
#ifdef MEMPROF_DEBUG
#define TRACE(...) pr_info("memprof: " __VA_ARGS__)
#else
#define TRACE(...)
#endif

#define INFO(...) pr_info("memprof: " __VA_ARGS__)
#define ERR(...) pr_err("memprof: " __VA_ARGS__)


struct memprof_state_s {
	int tried_hv;
	int hv_fd;
	char cmd[16];
	int ncmd;
	bool wait_nl;
	struct memprof_result hv_results;
	struct mutex lock;   /* avoid overlapping write() calls */
};

static struct memprof_state_s memprof_state;

/*
 * Shorthand for making a control request to the hypervisor memprof driver.
 */
static inline int memprof_hv_request(u64 request)
{
	int empty = 0;

	return hv_dev_pwrite(memprof_state.hv_fd, 0, (HV_VirtAddr)&empty,
			     sizeof(empty), request);
}

#define MEMPROF_DEV_COUNT 1

static long memprof_ioctl(struct file *filp,
			  unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case MEMPROF_IOCTL_START:
		memprof_hv_request(MEMPROF_START_OFF);
		break;

	case MEMPROF_IOCTL_STOP:
		memprof_hv_request(MEMPROF_STOP_OFF);
		break;

	case MEMPROF_IOCTL_CLEAR:
		memprof_hv_request(MEMPROF_CLEAR_OFF);
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

#ifdef CONFIG_COMPAT
static long memprof_compat_ioctl(struct file *filp,
				 unsigned int a, unsigned long b)
{
	/* Sign-extend the argument so it can be used as a pointer. */
	return memprof_ioctl(filp, a, (int)(long)b);
}
#endif

/*
 * Accept commands via strings written to /proc/file/memprof; this is a bit
 * easier than the ioctl's when using from a script.  "start", "stop", and
 * "clear" do the obvious actions.  Each must be followed by a newline.
 */
static ssize_t memprof_write(struct file *filp, const char __user *buf,
			     size_t count, loff_t *f_pos)
{
	int numleft = count;

	mutex_lock(&memprof_state.lock);

	while (numleft--) {
		char c;

		if (get_user(c, buf++)) {
			mutex_unlock(&memprof_state.lock);
			return -EFAULT;
		}

		if (memprof_state.wait_nl && c != '\n')
			continue;

		if (c == '\n') {
			memprof_state.wait_nl = false;
			memprof_state.cmd[memprof_state.ncmd] = '\0';
			memprof_state.ncmd = 0;

			if (!strcmp(memprof_state.cmd, "start"))
				memprof_hv_request(MEMPROF_START_OFF);
			else if (!strcmp(memprof_state.cmd, "stop"))
				memprof_hv_request(MEMPROF_STOP_OFF);
			else if (!strcmp(memprof_state.cmd, "clear"))
				memprof_hv_request(MEMPROF_CLEAR_OFF);
			else {
				mutex_unlock(&memprof_state.lock);
				return -ENOTTY;
			}
		} else if (memprof_state.ncmd >=
			   sizeof(memprof_state.cmd) - 1) {
			memprof_state.wait_nl = true;
			memprof_state.ncmd = 0;
		} else {
			memprof_state.cmd[memprof_state.ncmd++] = c;
		}
	}

	mutex_unlock(&memprof_state.lock);
	return count;
}

/*
 * Start a new run through the file.  We update our HV stats at this
 * point so that the user can seek-to-0 and reread instead of closing,
 * reopening, and then reading.
 *
 * Note that if we didn't test for "*pos == 0" here, each userspace
 * read() call would re-latch a new set of values, which would cause
 * inconsistent reporting for the different lines of output.
 * But seeking within the file will cause it to relatch the values,
 * so that's a bad idea, other than seeking back to offset zero.
 */
static void *memprof_seq_start(struct seq_file *s, loff_t *pos)
{
	if (*pos == 0) {
		int err = hv_dev_pread(memprof_state.hv_fd, 0,
				       (HV_VirtAddr)&memprof_state.hv_results,
				       sizeof(memprof_state.hv_results),
				       MEMPROF_RESULT_OFF);

		if (err < 0) {
			ERR("hv_dev_pread() failed with %d\n", err);
			return ERR_PTR(-EIO);
		}
	}

	if (*pos >= MEMPROF_MAX_MSHIMS)
		return NULL;
	return &memprof_state.hv_results.stats[*pos];
}

static void *memprof_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	if (*pos >= MEMPROF_MAX_MSHIMS)
		return NULL;
	return &memprof_state.hv_results.stats[*pos];
}

static void memprof_seq_stop(struct seq_file *s, void *v)
{
}

/* format as text with aligned columns */
static int memprof_seq_show(struct seq_file *s, void *v)
{
	struct memprof_stats *stats = (struct memprof_stats *)v;
	struct memprof_result *hv_results = &memprof_state.hv_results;
	const char *row1 = "%-26s%21llu\n";		/* left-aligned */
	const char *rows = "shim%d%-21s%21llu\n";
	int i = stats - &hv_results->stats[0];
	u64 total;

	if (i == 0)
		seq_printf(s, row1, "cycles:", hv_results->cycles);

	if (!stats->is_valid)
		return 0;

	total = stats->read_hit_count +
		stats->read_miss_count +
		stats->write_hit_count +
		stats->write_miss_count;
	seq_printf(s, rows, i, "_read_hit_count:", stats->read_hit_count);
	seq_printf(s, rows, i, "_read_miss_count:", stats->read_miss_count);
	seq_printf(s, rows, i, "_write_hit_count:", stats->write_hit_count);
	seq_printf(s, rows, i, "_write_miss_count:", stats->write_miss_count);
	seq_printf(s, rows, i, "_op_count:", total);
	seq_printf(s, rows, i, "_read_latency_count:", stats->lrd_count);
	seq_printf(s, rows, i, "_read_latency_cycles:", stats->lrd_cycles);
	seq_printf(s, "\n");

	return 0;
}

static const struct seq_operations memprof_seq_ops = {
	.start = memprof_seq_start,
	.next = memprof_seq_next,
	.stop = memprof_seq_stop,
	.show = memprof_seq_show,
};

static int memprof_open(struct inode *inode, struct file *filp)
{
	/* See whether we've got a device to attach to. */
	if (!memprof_state.tried_hv) {
		memprof_state.hv_fd = hv_dev_open((HV_VirtAddr)hv_filename, 0);
		memprof_state.tried_hv = 1;
	}
	if (memprof_state.hv_fd < 0) {
		INFO("HV memprof interface is not available\n");
		return -ENXIO;
	}

	/* We use a seq_file to handle ASCII-formatted reads. */
	return seq_open(filp, &memprof_seq_ops);
}

static const struct file_operations memprof_fops = {
	.open = memprof_open,
	.read = seq_read,
	.write = memprof_write,
	.llseek = seq_lseek,
	.release = seq_release,
	.unlocked_ioctl = memprof_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = memprof_compat_ioctl,
#endif
};


static int __init memprof_init(void)
{
	struct proc_dir_entry *entry;

	TRACE("Initializing memprof driver\n");

	/* We defer opening the HV device until open(). */
	memprof_state.tried_hv = 0;
	memprof_state.hv_fd = -1;
	memprof_state.ncmd = 0;
	memprof_state.wait_nl = false;
	mutex_init(&memprof_state.lock);

	/* We need one character device. */
	entry = create_proc_entry("tile/memprof", 0444, NULL);
	if (entry)
		entry->proc_fops = &memprof_fops;

	return 0;
}

late_initcall(memprof_init);
