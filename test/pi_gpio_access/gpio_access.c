/**
 * Access Raspberry Pi 3 GPIO pin16 via kernel module.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>

#define BCM2837_PERI_BASE 	0x3F000000 //phyiscal address of Pi's peripheral 
#define GP_BASE 			(BCM2837_PERI_BASE + 0x200000)
#define MAP_SIZE 			(4*1024)

void *gpio_map;
volatile unsigned *gpio;

void gpio_input(int pin) {

	*(gpio + (pin / 10)) &= ~(7 << ((pin % 10) * 3));

}

void gpio_output(int pin) {

	*(gpio + (pin / 10)) |=  (1 << ((pin % 10) * 3));

}

void set_gpio(int pin) {

	*(gpio + 7) = 1 << pin;

}

int __init gp_init(void) {

	printk(KERN_ALERT "Module gpio_access inserted \n");

	gpio_map = ioremap(GP_BASE, MAP_SIZE);

	gpio = (volatile unsigned *)gpio_map;

	gpio_input(16);
	gpio_output(16);
	set_gpio(16);

	return 0;

}

void __exit gp_exit(void) {

	printk(KERN_ALERT "Module gpio_access closed \n");
	iounmap(gpio_map);

}


MODULE_LICENSE("GPL");

module_init(gp_init);
module_exit(gp_exit);
