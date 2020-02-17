#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <mach/system.h>

int rb3011_vreg_rxdata(struct i2c_client *client, char *rxdata, int length) {
    int rc = 0;
    unsigned short saddr = client->addr;
    struct i2c_msg msgs[] = {
	{
	    .addr  = saddr,
	    .flags = 0,
	    .len   = 1,
	    .buf   = (unsigned char *) rxdata,
	},
	{
	    .addr  = saddr,
	    .flags = I2C_M_RD,
	    .len   = length,
	    .buf   = (unsigned char *) rxdata,
	},
    };
    rc = i2c_transfer(client->adapter, msgs, 2);
    if (rc < 0) printk("rb3011_vreg_rxdata failed 0x%x (%d)\n", saddr, rc);
    return rc;
}

int rb3011_vreg_txdata(struct i2c_client *client, char *txdata, int length) {
    int rc = 0;
    unsigned short saddr = client->addr;
    struct i2c_msg msg[] = {
	{
	    .addr = saddr,
	    .flags = 0,
	    .len = length,
	    .buf = (unsigned char *) txdata,
	},
    };
    rc = i2c_transfer(client->adapter, msg, 1);
    if (rc < 0) printk("rb3011_vreg_txdata faild 0x%x (%d)\n", saddr, rc);
    return rc;
}

static char convert_voltage(int uV) {
    if (uV < 680000) {
	return 0x00;
    }
    else if (uV > 1950000) {
	return 0x7f;
    }
    else {
	return (uV - 680000) / 10000;
    }
}

struct i2c_client *vreg_client = NULL;
// core0 voltage for UBI32, CX, PCIe etc
// core1 voltage for Krait APC cores
void rb3011_set_core_voltage(int core, int uV) {
    if (core != 0 && core != 1) {
	printk("rb3011_vreg: invalid cpu core index (%d)\n", core);
    }
    else if (vreg_client == NULL) {
	printk("rb3011_vreg: vreg i2c client is NULL\n");
    }
    else if (uV > 1950000) {
	printk("rb3011_vreg: voltage %d too high for akronite\n", uV);
    }
    else {
	unsigned char data[2] = { core, convert_voltage(uV) | BIT(7) };
	rb3011_vreg_txdata(vreg_client, data, sizeof(data));
    }
}

struct class_attribute_core {
    struct class_attribute class_attr;
    int core;
};

static ssize_t show_gpio_state(struct class *class,
			struct class_attribute *attr,
			char *output_buffer)
{
    return 0;
}

static ssize_t store_nothing(struct class *class,
			     struct class_attribute *attr,
			     const char *buf, size_t count)
{
    struct class_attribute_core *info;
    info = container_of(attr, struct class_attribute_core, class_attr);
    rb3011_set_core_voltage(info->core, simple_strtol(buf, NULL, 10));
    return count;
}

static struct class_attribute_core cac[2];
static void add_sysfs_core_voltage(struct class *hw_info,
				   const char *filename,
				   int core) {

    struct class_attribute_core *info = &cac[core];
    memset(info, 0, sizeof(struct class_attribute_core));

    info->class_attr.attr.name = filename;
    info->class_attr.attr.mode = S_IWUSR | S_IRUGO;
    info->class_attr.show = &show_gpio_state;
    info->class_attr.store = &store_nothing;
    info->core = core;

    if (class_create_file(hw_info, &info->class_attr) < 0) {
	printk("add_sysfs_core_voltage: failed\n");
    }
}

static int rb3011_vreg_probe(struct i2c_client *client,
			     const struct i2c_device_id *id) {
    static int sysfs_added = 0;
    vreg_client = client;
    if (!sysfs_added) {
	struct class *hw_info = class_create(THIS_MODULE, "hw_info");
	add_sysfs_core_voltage(hw_info, "core0_voltage", 0);
	add_sysfs_core_voltage(hw_info, "core1_voltage", 1);
	sysfs_added = 1;
    }
    rb3011_set_core_voltage(0, 1200000);
    return 0;
}

static int rb3011_vreg_remove(struct i2c_client *client) {
    vreg_client = NULL;
    return 0;
}

static const struct i2c_device_id rb3011_vreg_id[] = {
    { "rb3011_vreg", 0 },
    { },
};

static struct i2c_driver rb3011_vreg_driver = {
    .id_table = rb3011_vreg_id,
    .probe  = rb3011_vreg_probe,
    .remove = rb3011_vreg_remove,
    .driver = { .name = "rb3011_vreg", },
};

static int __init rb3011_vreg_add_driver(void) {
    if (rb_mach != RB_MACH_IPQ806X) {
	return -1;
    }
    return i2c_add_driver(&rb3011_vreg_driver);
}

subsys_initcall(rb3011_vreg_add_driver);
