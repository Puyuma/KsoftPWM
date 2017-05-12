/**
 * Cyclic latency test for real-time driver delay function in rapberry pi3 b+.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include "sysfs_print.h"
#include <linux/init.h>
#include <linux/module.h>
#include <rtdm/driver.h> //for RTDM function

#define TASK_PRIO 99
#define array_size 10000
#define INTERVAL (end - start)

/* latency test variable */
int times = 0;
int test_period = 0;

/* latency test measurement variable */
int negative_num = 0;
int overrun_num = 0;
int latency[array_size] = {0};
int64_t max_latency = 0;
int64_t min_latency = LONG_MAX;

/* real-time time measure function variable */
rtdm_task_t rt_latency_task;
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

static struct kobject *kobj_rt_latency;
BIN_ATTR_RO(lat_test, BUFF_SIZE);

void latency_task(void *arg)
{

	int i;
	int64_t latency_tmp;
	static int buf_count = 0;

	rtdm_printk("rt_latency: Cyclic latency test start.\n");

	for (i = 0; i < times; i++) {

		/* Use rtdm_task_sleep function to measure latency */
		start = rtdm_clock_read();
		rtdm_task_sleep(test_period * 1000);
		end = rtdm_clock_read();

		latency_tmp = INTERVAL - test_period * 1000;

		if (latency_tmp < 0) {
			negative_num++;
		} else {
			/* Use do_div function to divid the 64-bits int */
			do_div(latency_tmp, 1000);

			if (latency_tmp > array_size - 1)
				overrun_num++;
			else
				latency[latency_tmp]++;

			if (latency_tmp < min_latency)
				min_latency = latency_tmp;

			if (latency_tmp > max_latency)
				max_latency = latency_tmp;
		}
	}

	/* Deal with the measurement results and send to sysfs */
	buf_count =
	    sprintf(gb_buf, "# Max:%lld Min:%lld Overrun:%d Negative:%d\n",
		    max_latency, min_latency, overrun_num, negative_num);

	if (max_latency < array_size) {
		for (i = 0; i <= max_latency; i++) {
			if (buf_count < BUFF_SIZE)
				buf_count += sprintf(gb_buf + buf_count,
						     "%d %d\n", i, latency[i]);
			else
				break;
		}
	} else {
		for (i = 0; i < array_size; i++) {
			if (buf_count < BUFF_SIZE)
				buf_count += sprintf(gb_buf + buf_count,
						     "%d %d\n", i, latency[i]);
			else
				break;
		}
	}

	rtdm_printk("rt_latency: data computation finished.\n");
}

static int __init rt_latency_init(void)
{
	int err;

	/* register the sysfs interface */
	kobj_rt_latency = kobject_create_and_add("rt_latency", kernel_kobj);
	if (!kobj_rt_latency)
		return -ENOMEM;

	err = sysfs_create_bin_file(kobj_rt_latency, &bin_attr_lat_test);
	if (err)
		rtdm_printk("failed to create the sysfs file in "
			    "/sys/kernel/kernel_latency \n");

	/* cyclic latency test */
	if (times == 0) {
		times = 10;
	}

	if (test_period == 0) {
		test_period = 100;
	}

	err = rtdm_task_init(&rt_latency_task, "rt_error_task", latency_task,
			     NULL, TASK_PRIO, 0);
	if (err) {
		printk(KERN_WARNING
		       "failed to create rt_error_task on rtdm_task_init\n");
	}

	return 0;
}

void __exit rt_latency_exit(void)
{
	rtdm_printk("rt_latency: Cyclic latency test finished.\n");
	rtdm_task_destroy(&rt_latency_task);
	kobject_put(kobj_rt_latency);
}

module_init(rt_latency_init);
module_exit(rt_latency_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wang Shao-Hua");
