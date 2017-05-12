/**
 * Print out the cyclic test result by sysfs interface.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#ifndef _SYSFSPRINT_H_
#define _SYSFSPRINT_H_

#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define BUFF_SIZE PAGE_SIZE * 2

char buff[BUFF_SIZE];
char *gb_buf = buff;

static ssize_t lat_test_read(struct file *, struct kobject *,
			     struct bin_attribute *, char *, loff_t, size_t);

#endif
