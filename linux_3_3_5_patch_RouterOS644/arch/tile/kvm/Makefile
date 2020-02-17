#
# Makefile for Kernel-based Virtual Machine module
#

EXTRA_CFLAGS += -Ivirt/kvm -Iarch/tile/kvm

kvm-y += $(addprefix ../../../virt/kvm/, kvm_main.o)

kvm-y += kvm-tile.o

obj-$(CONFIG_KVM) += kvm.o
