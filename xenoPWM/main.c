/**
 * RTDM device for software PWM based on Xenomai 3.0.3 in Rapberry Pi3 b+.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>		//for kzalloc, kfree
#include <rtdm/driver.h>	//for RTDM function

#include "gpio.h"
#include "xenopwm.h"

#define PWM_RANGE	(duty_array[pos].max - duty_array[pos].min)
#define HIGH_TIME	(PERIOD_NS / PWM_RANGE * duty_array[pos].duty_cycle)
#define LOW_TIME	(PERIOD_NS / PWM_RANGE *(PWM_RANGE - duty_array[pos].duty_cycle))

rtdm_task_t task_array[MAX_PWM_NUM];
int pwm_pin_count = 0;
Dutycycle *duty_array;
void *gpio_map;
volatile unsigned *gpio;
char* task_name[5] = {"pwmtask_1", "pwmtask_2", "pwmtask_3", "pwmtask_4", "pwmtask_5"};

void pwm_task(void *arg) {

	int pos;

	pos = (int)arg;

	rtdm_printk("pwm_task for pin %d start\n", duty_array[pos].pin);

	while(!rtdm_task_should_stop()) {

		gpio_set(duty_array[pos].pin);
		rtdm_task_sleep(HIGH_TIME);	
		gpio_clr(duty_array[pos].pin);
		rtdm_task_sleep(LOW_TIME);
	}
}

static int xeno_pwm_ioctl(struct rtdm_fd *fd, unsigned int cmd, void __user *arg) {

	int err = 0, i, pos;
	int *tmp;
	int pin_exist = 0;

	tmp = NULL;

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

			duty_array[pwm_pin_count].pin = tmp[0];
			duty_array[pwm_pin_count].min = tmp[1];
			duty_array[pwm_pin_count].max = tmp[2];
			duty_array[pwm_pin_count].duty_cycle = 10;

			rtdm_printk(KERN_INFO "%d, %d, %d, %d, %s\n", duty_array[pwm_pin_count].pin, duty_array[pwm_pin_count].min, duty_array[pwm_pin_count].max, duty_array[pwm_pin_count].duty_cycle, task_name[pwm_pin_count]);

			//create RTDM task for pwm task
			err = rtdm_task_init(&task_array[pwm_pin_count], task_name[pwm_pin_count], pwm_task, (void *)pwm_pin_count, TASK_PRIO, 0);
			if(err) {
				printk(KERN_WARNING "failed to create pwm task on rtdm_task_init %d\n", pwm_pin_count);
				break;
			}

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

static struct rtdm_driver xeno_pwm = {

	.profile_info = RTDM_PROFILE_INFO(xenopwm, RTDM_CLASS_EXPERIMENTAL, RTDM_SUBCLASS_GENERIC, XENOPWM_VERSION),
	.device_flags = RTDM_NAMED_DEVICE,
	.device_count = 10,
	.ops = {
		.ioctl_nrt = xeno_pwm_ioctl,
	},
};

static struct rtdm_device xeno_pwm_dev = {
	.driver = &xeno_pwm,
	.label = PWM_DRIVER,
};

int __init xeno_pwm_init(void) {

	int ret = 0;

	printk(KERN_INFO "XenoPWM module load.\n");

	gpio_map = ioremap(GP_BASE, MAP_SIZE);
	gpio = (volatile unsigned *)gpio_map;

	//allocate duty_array memory
	duty_array = kzalloc(sizeof(Dutycycle) * MAX_PWM_NUM, GFP_KERNEL);
	if(!(duty_array)) {
		printk(KERN_WARNING "failed to allocate duty_array memory by kzalloc\n");
	}

	//register RTDM device
	ret = rtdm_dev_register(&xeno_pwm_dev);
	if(ret) {
		printk(KERN_WARNING "failed to register RTDM device for xenopwm\n");
	}

	return 0;

}

void __exit xeno_pwm_exit(void) {

	int i;

	printk(KERN_INFO "XenoPWM module unload.\n");

	for(i = 0; i < pwm_pin_count; i++) {
		rtdm_task_destroy(&task_array[i]);
	}
	
	iounmap(gpio_map);

	rtdm_dev_unregister(&xeno_pwm_dev);

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wang Shao-Hua");

module_init(xeno_pwm_init);
module_exit(xeno_pwm_exit);
