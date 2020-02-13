#include <linux/clk.h>
#include <linux/export.h>
#include <mach/system.h>

struct clk_hw;

int clk_set_rate_ak(struct clk *clk, unsigned long rate);
int clk_prepare_ak(struct clk *clk);
void clk_unprepare_ak(struct clk *clk);
int clk_set_parent_ak(struct clk *clk, struct clk *parent);
long clk_round_rate_ak(struct clk *clk, unsigned long rate);
struct clk *clk_get_parent_ak(struct clk *clk);
unsigned long clk_get_rate_ak(struct clk *clk);
void clk_disable_ak(struct clk *clk);
int clk_enable_ak(struct clk *clk);

int __clk_get_dk(struct clk *clk);
void __clk_put_dk(struct clk *clk);
int clk_set_rate_dk(struct clk *clk, unsigned long rate);
struct clk *clk_register_dk(struct device *dev, struct clk_hw *hw);
void clk_unregister_dk(struct clk *clk);
int clk_prepare_dk(struct clk *clk);
void clk_unprepare_dk(struct clk *clk);
int clk_set_parent_dk(struct clk *clk, struct clk *parent);
struct clk *clk_get_parent_dk(struct clk *clk);
long clk_round_rate_dk(struct clk *clk, unsigned long rate);
unsigned long clk_get_rate_dk(struct clk *clk);
void clk_disable_dk(struct clk *clk);
int clk_enable_dk(struct clk *clk);

static int (*__clk_get_p)(struct clk *clk) = NULL;
static void (*__clk_put_p)(struct clk *clk) = NULL;
static int (*clk_set_rate_p)(struct clk *clk, unsigned long rate) = &clk_set_rate_ak;
static long (*clk_round_rate_p)(struct clk *clk, unsigned long rate) = &clk_round_rate_ak;
static unsigned long (*clk_get_rate_p)(struct clk *clk) = &clk_get_rate_ak;
static struct clk *(*clk_register_p)(struct device *dev, struct clk_hw *hw) = NULL;
static void (*clk_unregister_p)(struct clk *clk) = NULL;
static int (*clk_prepare_p)(struct clk *clk) = &clk_prepare_ak;
static void (*clk_unprepare_p)(struct clk *clk) = &clk_unprepare_ak;
static struct clk *(*clk_get_parent_p)(struct clk *clk) = &clk_get_parent_ak;
static int (*clk_set_parent_p)(struct clk *clk, struct clk *parent) = &clk_set_parent_ak;
static int (*clk_enable_p)(struct clk *clk) = &clk_enable_ak;
static void (*clk_disable_p)(struct clk *clk) = &clk_disable_ak;

int __clk_get(struct clk *clk) {
    return __clk_get_p ? __clk_get_p(clk) : 1;
}
EXPORT_SYMBOL(__clk_get);

void __clk_put(struct clk *clk) {
    if (__clk_put_p) __clk_put_p(clk);
}
EXPORT_SYMBOL(__clk_put);

int clk_set_rate(struct clk *clk, unsigned long rate) {
    return clk_set_rate_p ? clk_set_rate_p(clk, rate) : 0;
}
EXPORT_SYMBOL(clk_set_rate);

long clk_round_rate(struct clk *clk, unsigned long rate) {
    return clk_round_rate_p ? clk_round_rate_p(clk, rate) : 0;
}
EXPORT_SYMBOL(clk_round_rate);

unsigned long clk_get_rate(struct clk *clk) {
    return clk_get_rate_p ? clk_get_rate_p(clk) : 0;
}
EXPORT_SYMBOL(clk_get_rate);

struct clk *clk_register(struct device *dev, struct clk_hw *hw) {
    return clk_register_p ? clk_register_p(dev, hw) : NULL;
}
EXPORT_SYMBOL(clk_register);

void clk_unregister(struct clk *clk) {
    if (clk_unregister_p) clk_unregister_p(clk);
}
EXPORT_SYMBOL(clk_unregister);

int clk_prepare(struct clk *clk) {
    return clk_prepare_p ? clk_prepare_p(clk) : 0;
}
EXPORT_SYMBOL(clk_prepare);

void clk_unprepare(struct clk *clk) {
    if (clk_unprepare_p) clk_unprepare_p(clk);
}
EXPORT_SYMBOL(clk_unprepare);

int clk_set_parent(struct clk *clk, struct clk *parent) {
    return clk_set_parent_p ? clk_set_parent_p(clk, parent) : 0;
}
EXPORT_SYMBOL(clk_set_parent);

struct clk *clk_get_parent(struct clk *clk) {
    return clk_get_parent_p ? clk_get_parent_p(clk) : NULL;
}
EXPORT_SYMBOL(clk_get_parent);

int clk_enable(struct clk *clk) {
    return clk_enable_p ? clk_enable_p(clk) : 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk) {
    if (clk_disable_p) clk_disable_p(clk);
}
EXPORT_SYMBOL(clk_disable);

void init_qcom_clock(void) {
    if (rb_mach == RB_MACH_IPQ40XX) {
	__clk_get_p = __clk_get_dk;
	__clk_put_p = __clk_put_dk;
	clk_set_rate_p = clk_set_rate_dk;
	clk_register_p = clk_register_dk;
	clk_unregister_p = clk_unregister_dk;
	clk_prepare_p = clk_prepare_dk;
	clk_unprepare_p = clk_unprepare_dk;
	clk_set_parent_p = clk_set_parent_dk;
	clk_get_parent_p = clk_get_parent_dk;
	clk_round_rate_p = clk_round_rate_dk;
	clk_get_rate_p = clk_get_rate_dk;
	clk_enable_p = clk_enable_dk;
	clk_disable_p = clk_disable_dk;
    }
}
