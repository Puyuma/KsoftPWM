/**
 * Char device for software PWM in Raspberry Pi 3 b+.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>		//for kzalloc, kfree
#include <linux/kthread.h>	//for kthread_create, kthread_stop
#include <linux/uaccess.h>	//for access_ok
#include <linux/delay.h>	//for delay
#include <linux/io.h>

#include "gpio.h"
#include "softpwm.h"

#define PWM_RANGE	(duty_array[pos].max - duty_array[pos].min)
#define PERIOD_US	(1000000 / PWM_FREQ)
#define HIGH_TIME	(PERIOD_US * duty_array[pos].duty_cycle) / PWM_RANGE
#define LOW_TIME	(PERIOD_US * (PWM_RANGE - duty_array[pos].duty_cycle)) / PWM_RANGE

struct cdev *cdevp = NULL;
static struct task_struct **pwm_tsk;
int pwm_pin_count = 0;
Dutycycle *duty_array;
void *gpio_map;
volatile unsigned *gpio;
int dev_major;
int dev_minor = 0;
char* name[5] = {"pwmtask_1", "pwmtask_2", "pwmtask_3", "pwmtask_4", "pwmtask_5"};

int pwm_task(void *arg) {

	int pos;

	pos = (int)arg;

	for(;;) {
		if (kthread_should_stop()) break;

		gpio_set(duty_array[pos].pin);
		usleep_range(HIGH_TIME, HIGH_TIME);
		gpio_clr(duty_array[pos].pin);
		usleep_range(LOW_TIME, LOW_TIME);
	}

	return 0;
}

long pwm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {

	int err = 0, i, pos;
	int *tmp;
	int pin_exist = 0;

	tmp = NULL;
    
	if (_IOC_TYPE(cmd) != PWM_MAGIC_NUM) return -ENOTTY;
	if (_IOC_NR(cmd) > 4) return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	switch(cmd) {

		case PIN_SET:// 0 pin, 1 type(INPUT or OUTPUT)

			tmp = (int *)arg;

			if(tmp[1] != 0 && tmp[1] != 1) {
				printk(KERN_WARNING "wrong pin type input for PIN_SET!\n");
				break;
			}

			(tmp[1] == 0) ? gpio_input(tmp[0]) : gpio_output(tmp[0]);

			break;

		case GPIO_SET:// 0 pin, 1 status(HIGH or LOW)

			tmp = (int *)arg;

			if(tmp[1] != 0 && tmp[1] != 1) {
				printk(KERN_WARNING "wrong pin type input for GPIO_SET!\n");
				break;
			}

			(tmp[1] == 0) ? gpio_clr(tmp[0]) : gpio_set(tmp[0]);
			break;

		case PWM_INIT:// 0 pin, 1 min, 2 max

			tmp = (int *)arg;

			//kthread create for PWM_INIT
			pwm_tsk[pwm_pin_count] = kthread_create(pwm_task, (void *)pwm_pin_count, name[pwm_pin_count]);
			if(IS_ERR(pwm_tsk[pwm_pin_count])) {
				printk(KERN_WARNING "failed to create kthread %d\n", pwm_pin_count);
				err = PTR_ERR(pwm_tsk[pwm_pin_count]);
				pwm_tsk[pwm_pin_count] = NULL;
				break;
			}

			duty_array[pwm_pin_count].pin = tmp[0];
			duty_array[pwm_pin_count].min = tmp[1];
			duty_array[pwm_pin_count].max = tmp[2];
			duty_array[pwm_pin_count].duty_cycle = 0;

			printk(KERN_INFO "%d, %d, %d, %d", duty_array[pwm_pin_count].pin, duty_array[pwm_pin_count].min, duty_array[pwm_pin_count].max, duty_array[pwm_pin_count].duty_cycle);

			wake_up_process(pwm_tsk[pwm_pin_count]);
			pwm_pin_count++;
			break;

		case PWM_SET:// 0 pin, 1 duty cycle

			tmp = (int *)arg;

			for(i = 0; i < pwm_pin_count; i++) {
				if(duty_array[i].pin == tmp[0]) {
					pin_exist = 1;
					pos = i;
					break;
				}
			}

			if(!(pin_exist)) {
				printk(KERN_WARNING "wrong pin input for PWM_SET!\n");
				pin_exist = 0;
				break;
			}

			duty_array[pos].duty_cycle = tmp[1];
			break;
	}

	return err;
}

struct file_operations pwm_fops = {
    .owner = THIS_MODULE,
	.unlocked_ioctl = pwm_ioctl
};

int __init pwm_init(void) {

	int result;
	dev_t dev = 0;

	printk(KERN_INFO "SoftPWM module load.\n");

	gpio_map = ioremap(GP_BASE, MAP_SIZE);
	gpio = (volatile unsigned *)gpio_map;

	//allocate duty_array memory
	duty_array = kzalloc(sizeof(Dutycycle) * MAX_PWM_NUM, GFP_KERNEL);
	if(!(duty_array)) {
		printk(KERN_WARNING "failed to allocate duty_array memory by kzalloc\n");
	}

	//allocate pwm_tsk array memory
	pwm_tsk = kzalloc(sizeof(struct task_struct *) * MAX_PWM_NUM, GFP_KERNEL);
	if(!(pwm_tsk)) {
		printk(KERN_WARNING "failed to allocate pwm_tsk array memory by kzalloc\n");
	}

	//register device number 
	result = alloc_chrdev_region(&dev, dev_minor, 1, DEVICE_NAME);
	if(result < 0) {
		printk(KERN_WARNING "failed to allocate char device for softPWM\n");
		return result;
	}

	//get the major number from dynamic allocate
	dev_major = MAJOR(dev);

	printk(KERN_INFO "register char device %s with %d : %d\n", DEVICE_NAME, dev_major, dev_minor);

	//allocate char device memory
	cdevp = kzalloc(sizeof(struct cdev), GFP_KERNEL); //GFP_KERNEL to allocate the normal memory
	if(!(cdevp)) {
		printk(KERN_WARNING "failed to allocate device memory by kzalloc\n");
		goto failed;
	}

	//initial and add the char device
	cdev_init(cdevp, &pwm_fops);
	cdevp->owner = THIS_MODULE;
	
	result = cdev_add(cdevp, dev, 1);
	if(result < 0) {
		printk(KERN_WARNING "failed to add char device %s\n", DEVICE_NAME);
		goto failed;
	}

	return 0;

failed:
	if(cdevp) {
		kfree(cdevp);
		cdevp = NULL;
	}

	return 0;
}

void __exit pwm_exit(void) {

	int i;
	dev_t dev;

	printk(KERN_INFO "SoftPWM module unload.\n");

	for(i = 0; i < pwm_pin_count; i++) {
		kthread_stop(pwm_tsk[i]);
	}

	iounmap(gpio_map);

	dev = MKDEV(dev_major, dev_minor);
	unregister_chrdev_region(dev, 1);

	kfree(cdevp);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wang Shao-Hua");

module_init(pwm_init);
module_exit(pwm_exit);
