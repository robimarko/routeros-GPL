/* * Copyright (c) 2013 Qualcomm Atheros, Inc. * */

/*
 * rpm-debug.c
 *     DebugFS support for RPM
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <mach/rpm.h>

#define MAX_REQUEST_LEN 16

static struct dentry *debugfs_base;

/*
 * rpm_debug_resource_get()
 * 	Generic API to get the value for a RPM resource
 *
 * NOTE This is just a placeholder for now
 */
static int rpm_debug_resource_get(void *data, u64 *val)
{
	*val = 0;
	return 0;
}

/*
 * rpm_debug_status_get()
 * 	Generic API to get the status for a RPM command request
 *
 * NOTE This is just a placeholder for now
 */
static int rpm_debug_status_get(void *data, u64 *val)
{
	*val = 0;
	return 0;
}

/*
 * rpm_debug_resource_set()
 * 	Generic API to set a RPM resource to particular value
 *
 * TODO Currently it only accepts only one 32-bit value as parameter and
 * sets all the value parameters in the RPM request to this value.
 * This should be extended to accept multiple values
 */
static int rpm_debug_resource_set(void *data, u64 val)
{
	int i;
	int ret;

	struct msm_rpm_iv_pair req[MAX_REQUEST_LEN];
	struct msm_rpm_map_data *resource;

	resource = data;

	for (i = 0; i < resource->count; i++) {
		req[i].id = resource->id;
		req[i].value = (uint32_t) val;
	}

	ret = msm_rpm_set(MSM_RPM_CTX_SET_0, req, resource->count);

	if (ret) {
		pr_err("rpm_debug_resource_set(%d, %d) failed (%d)\n", resource->id,
				(uint32_t)val, ret);
	}

	return ret;
}

/*
 * rpm_debug_idle_set()
 * 	Send command to RPM to set idle-state
 */
static int rpm_debug_idle_set(void *data, u64 val)
{
	int ret;
	// struct msm_rpm_iv_pair req;
	// struct msm_rpm_map_data *resource;

	ret = msm_rpm_send_idle_command();

	if (ret) {
		pr_err("rpm_debug_idle_set failed (%d)\n", ret);
	}

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(generic_resource_fops, rpm_debug_resource_get,
		rpm_debug_resource_set, "%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(command_fops, rpm_debug_status_get,
		rpm_debug_idle_set, "%llu\n");

/*
 * rpm_debug_resource_add()
 * 	Add a debugfs entry for a RPM resource
 */
static int rpm_debug_resource_add(struct msm_rpm_map_data *resource)
{
	struct dentry *resource_dir;
	char temp[50];

	strlcpy(temp, resource->dbg_name, ARRAY_SIZE(temp));
	resource_dir = debugfs_create_dir(temp, debugfs_base);

	if (!resource_dir) {
		return -ENOMEM;
	}

	if (!debugfs_create_file("value", S_IRUGO | S_IWUSR, resource_dir,
				resource, &generic_resource_fops)) {
		goto error;
	}

	return 0;
error:
	debugfs_remove_recursive(resource_dir);
	return -ENOMEM;
}

/*
 * rpm_debug_init()
 * 	Initialize RPM debugfs
 */
int __init rpm_debug_init(struct msm_rpm_platform_data *data)
{
	int i;
	struct msm_rpm_map_data *resource;

	resource = NULL;

	debugfs_base = debugfs_create_dir("rpm-dbg", NULL);

	if (!debugfs_base) {
		return -ENOMEM;
	}

	for (i = 0; i < MSM_RPM_ID_LAST;) {
		resource = &data->target_id[i];
		rpm_debug_resource_add(resource);
		i += (resource->count) ? resource->count : 1;
	}

	resource = &data->target_id[MSM_RPM_ID_ENTER_IDLE];
	if (!debugfs_create_file("enable_idle_state", S_IRUGO | S_IWUSR, debugfs_base,
				resource, &command_fops)) {
		pr_err("rpm_debug_init failed to create debugfs for enable_idle \n");
	}

	return 0;
}
