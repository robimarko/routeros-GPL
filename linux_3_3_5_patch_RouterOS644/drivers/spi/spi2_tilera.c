#define spidelay(nsecs) __insn_mf(); ndelay(nsecs)
#define DRIVER_NAME	"ccr-mmc-spi"
#define	SPI_MISO_GPIO	59
#define	SPI_MOSI_GPIO	55
#define	SPI_SCK_GPIO	54
#define	SPI_N_CHIPSEL	56
#include "spi-gpio.c"
