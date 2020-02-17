#
# Makefile for the Tile-Gx device access support.
#

obj-$(CONFIG_TILE_GXIO) += iorpc_globals.o kiorpc.o
obj-$(CONFIG_TILE_GXIO_DMA) += dma_queue.o
obj-$(CONFIG_TILE_GXIO_MPIPE) += mpipe.o iorpc_mpipe.o iorpc_mpipe_info.o
obj-$(CONFIG_TILE_GXIO_GPIO) += gpio.o iorpc_gpio.o
obj-$(CONFIG_TILE_GXIO_TRIO) += trio.o iorpc_trio.o
obj-$(CONFIG_TILE_GXIO_USB_HOST) += usb_host.o iorpc_usb_host.o
obj-$(CONFIG_TILE_GXIO_PKA) += pka.o iorpc_pka.o
obj-$(CONFIG_TILE_GXIO_MICA) += mica.o iorpc_mica.o
obj-$(CONFIG_TILE_GXIO_UART) += uart.o iorpc_uart.o
obj-$(CONFIG_TILE_GXIO_GXCR) += aead.o crypto.o token_bulk_crypto_digest.o \
				token_hmac_precalc.o token_aead.o
