/*
 *  linux/include/linux/clk-provider.h
 *
 *  Copyright (c) 2010-2011 Jeremy Kerr <jeremy.kerr@canonical.com>
 *  Copyright (C) 2011-2012 Linaro Ltd <mturquette@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __LINUX_CLK_PROVIDER_H
#define __LINUX_CLK_PROVIDER_H

#include <linux/io.h>
#include <linux/of.h>

/*
 * flags used across common struct clk.  these flags should only affect the
 * top-level framework.  custom flags for dealing with hardware specifics
 * belong in struct clk_foo
 */
#define CLK_SET_RATE_GATE	BIT(0) /* must be gated across rate change */
#define CLK_SET_PARENT_GATE	BIT(1) /* must be gated across re-parent */
#define CLK_SET_RATE_PARENT	BIT(2) /* propagate rate change up one level */
#define CLK_IGNORE_UNUSED	BIT(3) /* do not gate even if unused */
				/* unused */
#define CLK_IS_ROOT		BIT(4) /* Deprecated: Don't use */
#define CLK_IS_BASIC		BIT(5) /* Basic clk, can't do a to_clk_foo() */
#define CLK_GET_RATE_NOCACHE	BIT(6) /* do not use the cached clk rate */
#define CLK_SET_RATE_NO_REPARENT BIT(7) /* don't re-parent on rate change */
#define CLK_GET_ACCURACY_NOCACHE BIT(8) /* do not use the cached clk accuracy */
#define CLK_RECALC_NEW_RATES	BIT(9) /* recalc rates after notifications */
#define CLK_SET_RATE_UNGATE	BIT(10) /* clock needs to run to set rate */
#define CLK_IS_CRITICAL		BIT(11) /* do not gate, ever */
/* parents need enable during gate/ungate, set rate and re-parent */
#define CLK_OPS_PARENT_ENABLE	BIT(12)

struct clk_hw;
struct dentry;

struct clk {
	const char		*name;
	const struct clk_ops	*ops;
	struct clk_hw		*hw;
	struct module		*owner;
	struct clk		*parent;
	const char		**parent_names;
	struct clk		**parents;
	u8			num_parents;
	u8			new_parent_index;
	u8			safe_parent_index;
	unsigned long		rate;
	unsigned long		new_rate;
	struct clk		*safe_parent;
	struct clk		*new_parent;
	struct clk		*new_child;
	unsigned long		flags;
	unsigned int		enable_count;
	unsigned int		prepare_count;
	unsigned long		accuracy;
	struct hlist_head	children;
	struct hlist_node	child_node;
	unsigned int		notifier_count;
#ifdef CONFIG_DEBUG_FS
	struct dentry		*dentry;
#endif
	struct kref		ref;

};


/**
 * struct clk_rate_request - Structure encoding the clk constraints that
 * a clock user might require.
 *
 * @rate:		Requested clock rate. This field will be adjusted by
 *			clock drivers according to hardware capabilities.
 * @min_rate:		Minimum rate imposed by clk users.
 * @max_rate:		Maximum rate imposed by clk users.
 * @best_parent_rate:	The best parent rate a parent can provide to fulfill the
 *			requested constraints.
 * @best_parent_hw:	The most appropriate parent clock that fulfills the
 *			requested constraints.
 *
 */
struct clk_rate_request {
	unsigned long rate;
	unsigned long min_rate;
	unsigned long max_rate;
	unsigned long best_parent_rate;
	struct clk_hw *best_parent_hw;
};

/**
 * struct clk_ops -  Callback operations for hardware clocks; these are to
 * be provided by the clock implementation, and will be called by drivers
 * through the clk_* api.
 *
 * @prepare:	Prepare the clock for enabling. This must not return until
 *		the clock is fully prepared, and it's safe to call clk_enable.
 *		This callback is intended to allow clock implementations to
 *		do any initialisation that may sleep. Called with
 *		prepare_lock held.
 *
 * @unprepare:	Release the clock from its prepared state. This will typically
 *		undo any work done in the @prepare callback. Called with
 *		prepare_lock held.
 *
 * @is_prepared: Queries the hardware to determine if the clock is prepared.
 *		This function is allowed to sleep. Optional, if this op is not
 *		set then the prepare count will be used.
 *
 * @unprepare_unused: Unprepare the clock atomically.  Only called from
 *		clk_disable_unused for prepare clocks with special needs.
 *		Called with prepare mutex held. This function may sleep.
 *
 * @enable:	Enable the clock atomically. This must not return until the
 *		clock is generating a valid clock signal, usable by consumer
 *		devices. Called with enable_lock held. This function must not
 *		sleep.
 *
 * @disable:	Disable the clock atomically. Called with enable_lock held.
 *		This function must not sleep.
 *
 * @is_enabled:	Queries the hardware to determine if the clock is enabled.
 *		This function must not sleep. Optional, if this op is not
 *		set then the enable count will be used.
 *
 * @disable_unused: Disable the clock atomically.  Only called from
 *		clk_disable_unused for gate clocks with special needs.
 *		Called with enable_lock held.  This function must not
 *		sleep.
 *
 * @recalc_rate	Recalculate the rate of this clock, by querying hardware. The
 *		parent rate is an input parameter.  It is up to the caller to
 *		ensure that the prepare_mutex is held across this call.
 *		Returns the calculated rate.  Optional, but recommended - if
 *		this op is not set then clock rate will be initialized to 0.
 *
 * @round_rate:	Given a target rate as input, returns the closest rate actually
 *		supported by the clock. The parent rate is an input/output
 *		parameter.
 *
 * @determine_rate: Given a target rate as input, returns the closest rate
 *		actually supported by the clock, and optionally the parent clock
 *		that should be used to provide the clock rate.
 *
 * @set_parent:	Change the input source of this clock; for clocks with multiple
 *		possible parents specify a new parent by passing in the index
 *		as a u8 corresponding to the parent in either the .parent_names
 *		or .parents arrays.  This function in affect translates an
 *		array index into the value programmed into the hardware.
 *		Returns 0 on success, -EERROR otherwise.
 *
 * @get_parent:	Queries the hardware to determine the parent of a clock.  The
 *		return value is a u8 which specifies the index corresponding to
 *		the parent clock.  This index can be applied to either the
 *		.parent_names or .parents arrays.  In short, this function
 *		translates the parent value read from hardware into an array
 *		index.  Currently only called when the clock is initialized by
 *		__clk_init.  This callback is mandatory for clocks with
 *		multiple parents.  It is optional (and unnecessary) for clocks
 *		with 0 or 1 parents.
 *
 * @set_rate:	Change the rate of this clock. The requested rate is specified
 *		by the second argument, which should typically be the return
 *		of .round_rate call.  The third argument gives the parent rate
 *		which is likely helpful for most .set_rate implementation.
 *		Returns 0 on success, -EERROR otherwise.
 *
 * @set_rate_and_parent: Change the rate and the parent of this clock. The
 *		requested rate is specified by the second argument, which
 *		should typically be the return of .round_rate call.  The
 *		third argument gives the parent rate which is likely helpful
 *		for most .set_rate_and_parent implementation. The fourth
 *		argument gives the parent index. This callback is optional (and
 *		unnecessary) for clocks with 0 or 1 parents as well as
 *		for clocks that can tolerate switching the rate and the parent
 *		separately via calls to .set_parent and .set_rate.
 *		Returns 0 on success, -EERROR otherwise.
 *
 * @recalc_accuracy: Recalculate the accuracy of this clock. The clock accuracy
 *		is expressed in ppb (parts per billion). The parent accuracy is
 *		an input parameter.
 *		Returns the calculated accuracy.  Optional - if	this op is not
 *		set then clock accuracy will be initialized to parent accuracy
 *		or 0 (perfect clock) if clock has no parent.
 *
 * @get_phase:	Queries the hardware to get the current phase of a clock.
 *		Returned values are 0-359 degrees on success, negative
 *		error codes on failure.
 *
 * @set_phase:	Shift the phase this clock signal in degrees specified
 *		by the second argument. Valid values for degrees are
 *		0-359. Return 0 on success, otherwise -EERROR.
 *
 * @init:	Perform platform-specific initialization magic.
 *		This is not not used by any of the basic clock types.
 *		Please consider other ways of solving initialization problems
 *		before using this callback, as its use is discouraged.
 *
 * @debug_init:	Set up type-specific debugfs entries for this clock.  This
 *		is called once, after the debugfs directory entry for this
 *		clock has been created.  The dentry pointer representing that
 *		directory is provided as an argument.  Called with
 *		prepare_lock held.  Returns 0 on success, -EERROR otherwise.
 *
 *
 * The clk_enable/clk_disable and clk_prepare/clk_unprepare pairs allow
 * implementations to split any work between atomic (enable) and sleepable
 * (prepare) contexts.  If enabling a clock requires code that might sleep,
 * this must be done in clk_prepare.  Clock enable code that will never be
 * called in a sleepable context may be implemented in clk_enable.
 *
 * Typically, drivers will call clk_prepare when a clock may be needed later
 * (eg. when a device is opened), and clk_enable when the clock is actually
 * required (eg. from an interrupt). Note that clk_prepare MUST have been
 * called before clk_enable.
 */
struct clk_ops {
	int		(*prepare)(struct clk_hw *hw);
	void		(*unprepare)(struct clk_hw *hw);
	int		(*is_prepared)(struct clk_hw *hw);
	void		(*unprepare_unused)(struct clk_hw *hw);
	int		(*enable)(struct clk_hw *hw);
	void		(*disable)(struct clk_hw *hw);
	int		(*is_enabled)(struct clk_hw *hw);
	void		(*disable_unused)(struct clk_hw *hw);
	unsigned long	(*recalc_rate)(struct clk_hw *hw,
					unsigned long parent_rate);
	long		(*round_rate)(struct clk_hw *hw, unsigned long rate,
					unsigned long *parent_rate);
	long		(*determine_rate)(struct clk_hw *hw, unsigned long rate,
					unsigned long *best_parent_rate,
					struct clk **best_parent_clk);
	int		(*set_parent)(struct clk_hw *hw, u8 index);
	u8		(*get_parent)(struct clk_hw *hw);
	struct clk	*(*get_safe_parent)(struct clk_hw *hw);
	int		(*set_rate)(struct clk_hw *hw, unsigned long rate,
				    unsigned long parent_rate);
	int		(*set_rate_and_parent)(struct clk_hw *hw,
				    unsigned long rate,
				    unsigned long parent_rate, u8 index);
	unsigned long	(*recalc_accuracy)(struct clk_hw *hw,
					   unsigned long parent_accuracy);
	int		(*get_phase)(struct clk_hw *hw);
	int		(*set_phase)(struct clk_hw *hw, int degrees);
	void		(*init)(struct clk_hw *hw);
	int		(*debug_init)(struct clk_hw *hw, struct dentry *dentry);
};

/**
 * struct clk_init_data - holds init data that's common to all clocks and is
 * shared between the clock provider and the common clock framework.
 *
 * @name: clock name
 * @ops: operations this clock supports
 * @parent_names: array of string names for all possible parents
 * @num_parents: number of possible parents
 * @flags: framework-level hints and quirks
 */
struct clk_init_data {
	const char		*name;
	const struct clk_ops	*ops;
	const char		* const *parent_names;
	u8			num_parents;
	unsigned long		flags;
};

/**
 * struct clk_hw - handle for traversing from a struct clk to its corresponding
 * hardware-specific structure.  struct clk_hw should be declared within struct
 * clk_foo and then referenced by the struct clk instance that uses struct
 * clk_foo's clk_ops
 *
 * @core: pointer to the struct clk_core instance that points back to this
 * struct clk_hw instance
 *
 * @clk: pointer to the per-user struct clk instance that can be used to call
 * into the clk API
 *
 * @init: pointer to struct clk_init_data that contains the init data shared
 * with the common clock framework.
 */
struct clk_hw {
	void *core;
	struct clk *clk;
	const struct clk_init_data *init;
};

#define to_clk_hw(_clk) container_of(_clk, struct clk_hw, clk)

/*
 * DOC: Basic clock implementations common to many platforms
 *
 * Each basic clock hardware type is comprised of a structure describing the
 * clock hardware, implementations of the relevant callbacks in struct clk_ops,
 * unique flags for that hardware type, a registration function and an
 * alternative macro for static initialization
 */

/**
 * struct clk_fixed_rate - fixed-rate clock
 * @hw:		handle between common and hardware-specific interfaces
 * @fixed_rate:	constant frequency of clock
 */
struct clk_fixed_rate {
	struct		clk_hw hw;
	unsigned long	fixed_rate;
	unsigned long	fixed_accuracy;
	u8		flags;
};

struct regmap {
	unsigned regmap_phys_base;
	unsigned regmap_virt_base;
};

struct clk_regmap {
	struct clk_hw hw;
	struct regmap *regmap;
	unsigned int enable_reg;
	unsigned int enable_mask;
	bool enable_is_inverted;
};

#define to_clk_fixed_rate(_hw) container_of(_hw, struct clk_fixed_rate, hw)

struct clk *clk_register_fixed_rate(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned long fixed_rate);
struct clk_hw *clk_hw_register_fixed_rate(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned long fixed_rate);
struct clk *clk_register_fixed_rate_with_accuracy(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, unsigned long fixed_accuracy);
void clk_unregister_fixed_rate(struct clk *clk);
struct clk_hw *clk_hw_register_fixed_rate_with_accuracy(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, unsigned long fixed_accuracy);
void clk_hw_unregister_fixed_rate(struct clk_hw *hw);




struct clk *clk_register_dk(struct device *dev, struct clk_hw *hw);
struct clk *devm_clk_register(struct device *dev, struct clk_hw *hw);

void clk_unregister_dk(struct clk *clk);
void devm_clk_unregister(struct device *dev, struct clk *clk);

/* helper functions */
struct clk *clk_get_parent_by_index(struct clk *clk, u8 index);
struct clk *clk_get_parent_dk(struct clk *clk);

unsigned long clk_hw_get_rate(const struct clk_hw *hw);
unsigned long clk_get_flags(struct clk *clk);
unsigned long clk_hw_get_flags(const struct clk_hw *hw);
bool clk_hw_is_prepared(const struct clk_hw *hw);
bool clk_hw_is_enabled(const struct clk_hw *hw);
void clk_unprepare_dk(struct clk *clk);
int clk_notifier_register(struct clk *clk, struct notifier_block *nb);
int clk_notifier_unregister(struct clk *clk, struct notifier_block *nb);

struct clk *clk_get_by_name(const char *name);
void clk_hw_reparent(struct clk_hw *hw, struct clk_hw *new_parent);
void clk_hw_set_rate_range(struct clk_hw *hw, unsigned long min_rate,
			   unsigned long max_rate);

/*
 * FIXME clock api without lock protection
 */

struct of_device_id;

typedef void (*of_clk_init_cb_t)(struct device_node *);

struct clk_onecell_data {
	struct clk **clks;
	unsigned int clk_num;
};

struct clk_hw_onecell_data {
	unsigned int num;
	struct clk_hw *hws[];
};

extern struct of_device_id __clk_of_table;

#define CLK_OF_DECLARE(name, compat, fn) OF_DECLARE_1(clk, name, compat, fn)

unsigned long clk_hw_round_rate(struct clk_hw *hw, unsigned long rate);
long clk_get_accuracy(struct clk *clk);
unsigned long clk_get_rate_dk(struct clk *clk);

int clk_set_phase(struct clk *clk, int degrees);
int clk_get_phase(struct clk *clk);
bool clk_is_match(const struct clk *p, const struct clk *q);
int clk_prepare(struct clk *clk);
struct clk *clk_get(struct device *dev, const char *id);
struct clk *devm_clk_get(struct device *dev, const char *id);
void clk_put(struct clk *clk);
void devm_clk_put(struct device *dev, struct clk *clk);
bool clk_has_parent(struct clk *clk, struct clk *parent);
int clk_set_rate_range(struct clk *clk, unsigned long min, unsigned long max);
int clk_set_min_rate(struct clk *clk, unsigned long rate);
int clk_set_max_rate(struct clk *clk, unsigned long rate);
struct clk *clk_get_sys(const char *dev_id, const char *con_id);

/* */
void clk_disable_dk(struct clk *clk);
int clk_enable_dk(struct clk *clk);
int clk_set_parent_dk(struct clk *clk, struct clk *parent);
int clk_prepare_dk(struct clk *clk);
int clk_set_rate_dk(struct clk *clk, unsigned long rate);
unsigned long clk_round_rate_dk(struct clk *clk, unsigned long rate);


static inline int clk_prepare_enable_dk(struct clk *clk)
{
	int ret;

	ret = clk_prepare_dk(clk);
	if (ret)
		return ret;
	ret = clk_enable_dk(clk);
	if (ret)
		clk_unprepare_dk(clk);

	return ret;
}

static inline void clk_disable_unprepare_dk(struct clk *clk)
{
	clk_disable_dk(clk);
	clk_unprepare_dk(clk);
}

/*
 * wrap access to peripherals in accessor routines
 * for improved portability across platforms
 */

static inline u32 clk_readl(u32 __iomem *reg)
{
	return readl(reg);
}

static inline void clk_writel(u32 val, u32 __iomem *reg)
{
	writel(val, reg);
}

struct clk *clk_register_fixed_rate_with_accuracy(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, unsigned long fixed_accuracy);
struct clk *clk_register_fixed_rate(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned long fixed_rate);
#ifdef CONFIG_OF
int of_clk_add_provider(struct device_node *np,
	struct clk *(*clk_src_get)(struct of_phandle_args *args, void *data), void *data);
int of_clk_add_hw_provider(struct device_node *np, 
	struct clk_hw *(*get)(struct of_phandle_args *clkspec,  void *data), void *data);
void of_clk_del_provider(struct device_node *np);
struct clk *of_clk_src_simple_get(struct of_phandle_args *clkspec, void *data);
struct clk_hw *of_clk_hw_simple_get(struct of_phandle_args *clkspec, void *data);
struct clk *of_clk_src_onecell_get(struct of_phandle_args *clkspec, void *data);
struct clk_hw *of_clk_hw_onecell_get(struct of_phandle_args *clkspec, void *data);
unsigned int of_clk_get_parent_count(struct device_node *np);
int of_clk_parent_fill(struct device_node *np, const char **parents, unsigned int size);
int of_clk_detect_critical(struct device_node *np, int index, unsigned long *flags);
void of_clk_init(const struct of_device_id *matches);
void of_fixed_clk_setup(struct device_node *np);
void of_fixed_clk_setup(struct device_node *node);
struct clk *of_clk_get_by_name(struct device_node *np, const char *name);
#endif
#endif /* CLK_PROVIDER_H */
