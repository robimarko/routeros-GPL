KBUILD_CPPFLAGS += -Iarch/arm/mach-rb/include/al_hal

obj-y += common.o
obj-y += alpine.o
obj-y += al_fabric.o
obj-y += al_hal/
obj-y += alpine_pcie.o
obj-y += alpine_msix.o
obj-y += alpine_sd.o
obj-y += pci_error.o

obj-y += qcom.o clk-qcom.o
obj-$(CONFIG_QCOM_PCIE) += qcom_pcie.o qcom_pcie_irq.o
obj-$(CONFIG_ARCH_IPQ40XX) += ipq40xx/
obj-$(CONFIG_ARCH_IPQ806X) += ipq806x/
obj-$(CONFIG_ARCH_MVEBU) += mvebu/
