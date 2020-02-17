#ifndef _ARCH_TILE_GPIO_H
#define _ARCH_TILE_GPIO_H

#include <asm-generic/gpio.h>

#define gpio_get_value  __gpio_get_value
#define gpio_set_value  __gpio_set_value
#define gpio_cansleep   __gpio_cansleep
#define gpio_to_irq	__gpio_to_irq

#endif /* _ARCH_TILE_GPIO_H */
