#define spidelay(nsecs) __insn_mf(); ndelay(nsecs)
#define DRIVER_NAME	"ccr-mmc-spi-ccr1072"
#define	SPI_MISO_GPIO	47
#define	SPI_MOSI_GPIO	43
#define	SPI_SCK_GPIO	42
#define	SPI_N_CHIPSEL	44
#include "spi-gpio.c"
