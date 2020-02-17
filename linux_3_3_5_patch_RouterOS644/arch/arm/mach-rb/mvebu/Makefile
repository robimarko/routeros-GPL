ccflags-$(CONFIG_ARCH_MVEBU) := -I$(srctree)/$(src)/include \
	-I$(srctree)/arch/arm/plat-orion/include

AFLAGS_coherency_ll.o := -Wa,-march=armv7-a

obj-y += system-controller.o
obj-y += mvebu-soc-id.o
obj-y += cpu-reset.o
obj-y += board-v7.o
obj-y += coherency.o
obj-y += coherency_ll.o
obj-y += mvebu-mbus.o
obj-y += irq-armada-370-xp.o
obj-y += time.o
obj-y += cache-aurora-l2.o
obj-y += pmsu.o
obj-y += headsmp.o
obj-y += platsmp.o

obj-y += gpio-mvebu.o
obj-y += cpufreq.o
