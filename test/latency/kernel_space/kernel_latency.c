/**
 * Cyclic latency test for kernel space delay function in rapberry pi3 b+.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include "sysfs_print.h"
#include <linux/delay.h> //for delay
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <rtdm/driver.h> //for RTDM function

#define INTERVAL (end - start)
#define array_size 10000

/* latency test variable */
int times = 0;
int test_period = 0;

/* latency test measurement variable */
int negative_num = 0;
int overrun_num = 0;
int latency[array_size] = {0};
int64_t max_latency = 0;
int64_t min_latency = LLONG_MAX;

/* real-time time measure function variable */
nanosecs_abs_t start, end;

module_param(times, int, S_IRUGO | S_IWUSR);
module_param(test_period, int, S_IRUGO | S_IWUSR);

/**
  *      lat_test_read - read function when access the sysfs attribute
  *
  *      When access the /sys/kernel/kernel/latency attribute
  *      the lat_test_read function will act.
  */
static ssize_t lat_test_read(struct file *sys_file, struct kobject *kobj,
			     struct bin_attribute *attr, char *buf, loff_t pos,
			     size_t count)
{
	return sprintf(buf, gb_buf);
}

static struct kobject *kobj_latency;
BIN_ATTR_RO(lat_test, BUFF_SIZE);

static int __init kernel_latency_init(void)
{
	int i;
	int err;
	int64_t latency_tmp;
	static int buf_count = 0;

	printk("kernel_latency: Cyclic latency test start.\n");

	/* register the sysfs interface */
	kobj_latency = kobject_create_and_add("kernel_latency", kernel_kobj);
	if (!kobj_latency)
		return -ENOMEM;

	err = sysfs_create_bin_file(kobj_latency, &bin_attr_lat_test);
	if (err)
		printk("failed to create the sysfs file in "
		       "/sys/kernel/kernel_latency \n");

	/* cyclic latency test */
	if (times == 0) {
		times = 10;
	}

	if (test_period == 0) {
		test_period = 100;
	}

	for (i = 0; i < times; i++) {

		/* use usleep_range function to measure latency */
		start = rtdm_clock_read();
		usleep_range(test_period, test_period);
		end = rtdm_clock_read();

		latency_tmp = INTERVAL - test_period * 1000;
		do_div(latency_tmp, 1000);

		if (latency_tmp < 0)
			negative_num++;
		else if (latency_tmp > array_size - 1)
			overrun_num++;
		else
			latency[latency_tmp]++;

		if (latency_tmp < min_latency)
			min_latency = latency_tmp;

		if (latency_tmp > max_latency)
			max_latency = latency_tmp;
	}

	/* deal with the measurement results and send to sysfs*/
	buf_count =
	    sprintf(gb_buf, "# Max:%lld Min:%lld Overrun:%d Negative:%d\n",
		    max_latency, min_latency, overrun_num, negative_num);

	if (max_latency < array_size) {
		for (i = 0; i <= max_latency; i++) {
			if(buf_count < BUFF_SIZE)
				buf_count += sprintf(gb_buf + buf_count, "%d %d\n", i,
					     latency[i]);
			else
				break;
		}
	} else {
		for (i = 0; i < array_size; i++) {
			if(buf_count < BUFF_SIZE)
				buf_count += sprintf(gb_buf + buf_count, "%d %d\n", i,
                         latency[i]);
			else
				break;
		}
	}
	printk("kernel_latency: Data computation finished.");
	return 0;
}

void __exit kernel_latency_exit(void)
{
	printk("kernel_latency: Cyclic latency test finished.\n");
	kobject_put(kobj_latency);
}

module_init(kernel_latency_init);
module_exit(kernel_latency_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wang Shao-Hua");
