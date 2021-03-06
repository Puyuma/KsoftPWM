/**
 * Access Raspberry Pi 3 GPIO via kernel module.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#include <linux/io.h>

/* peripheral address of Pi3 */
#define BCM2837_PERI_BASE 0x3F000000
#define GP_BASE (BCM2837_PERI_BASE + 0x200000)
#define MAP_SIZE (4 * 1024)

void gpio_input(int pin);
void gpio_output(int pin);
void gpio_set(int pin);
void gpio_clr(int pin);

#endif
