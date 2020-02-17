###############################################################################
#  Jan 2007 Bruce Chang
#
#  Initial Release
#
#
#
###############################################################################


.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

obj-y    := init.o irq.o gpio.o

obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_MTK_MTD_NAND) +=  dev-nand.o
#obj-$(CONFIG_I2C_RALINK) +=  dev-i2c.o
