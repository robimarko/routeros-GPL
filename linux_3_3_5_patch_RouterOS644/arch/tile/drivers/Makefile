#
# Makefile for TILE drivers that don't belong elsewhere in the tree.
# Typically these are drivers that just present a simple char
# file_operations interface.
#
# For other Tilera-specific drivers, see:
#
#  drivers/char/tile-srom.c		SROM character driver
#  drivers/edac/tile_edac.c		EDAC driver
#  drivers/i2c/busses/i2c-tile.c	I2C driver
#  drivers/mtd/devices/tile_srom.c	SROM MTD driver
#  drivers/net/tile/			network
#  drivers/rtc/rtc-tile.c		RTC driver
#  drivers/tty/hvc/hvc_tile.c		hypervisor console
#  drivers/usb/host/ehci-tilegx.c	USB EHCI driver glue
#  drivers/usb/host/ohci-tilegx.c	USB OHCI driver glue
#  drivers/watchdog/tile_wdt.c		hypervisor watchdog
#

obj-$(CONFIG_TILE_BME_MEM)	+= bme_mem.o
obj-$(CONFIG_TILE_EEPROM)	+= eeprom.o
obj-$(CONFIG_TILE_HPI)		+= hpi.o
obj-$(CONFIG_TILE_MEMPROF)	+= memprof.o
obj-$(CONFIG_TILE_NETIO)	+= netio.o
obj-$(CONFIG_TILE_PKA)		+= pka.o
obj-$(CONFIG_TILE_RSHIM)	+= rshim.o
obj-$(CONFIG_TILE_SOFTUART)	+= softuart.o
obj-$(CONFIG_TILE_TMFIFO)	+= tmfifo.o
obj-$(CONFIG_TILE_USB)		+= usb.o
