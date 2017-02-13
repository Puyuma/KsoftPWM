/*
	Access raspberry pi3 gpio by kernel module.
	Shao-Hua, Wang
	2017.02.02
*/
#ifndef _GPIO_H_
#define _GPIO_H_

#include <linux/io.h>

#define BCM2837_PERI_BASE 	0x3F000000 //address of Pi3's peripheral 
#define GP_BASE 			(BCM2837_PERI_BASE + 0x200000)
#define MAP_SIZE 			(4*1024)

void gpio_input(int pin);
void gpio_output(int pin);
void gpio_set(int pin);
void gpio_clr(int pin);

#endif
