#include <linux/cpufreq.h>
#include <mach/system.h>

static int msys_cpufreq_verify(struct cpufreq_policy *policy)
{
	return 0;
}

static int msys_cpufreq_setpolicy(struct cpufreq_policy *policy)
{
	return 0;
}

static unsigned int msys_cpufreq_get_freq(unsigned int cpu)
{
	return 800000000 / 1000; // in kHz
}

static int __cpuinit msys_cpufreq_init(struct cpufreq_policy *policy)
{
	return 0;
}

static struct cpufreq_driver msys_cpufreq_driver = {
	.flags		= CPUFREQ_CONST_LOOPS,
	.init		= msys_cpufreq_init,
	.verify		= msys_cpufreq_verify,
	.setpolicy	= msys_cpufreq_setpolicy,
	.get		= msys_cpufreq_get_freq,
	.name		= "msys",
};

static int __init msys_cpufreq_register(void)
{
	if (rb_mach != RB_MACH_MSYS) {
		return 0;
	}

	return cpufreq_register_driver(&msys_cpufreq_driver);
}

late_initcall(msys_cpufreq_register);
