/**
 * Char device for software PWM in Raspberry pi3 b+.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include <linux/ioctl.h>

#define DEVICE_NAME "test"
#define DEVICE_ACCESS_NAME "datatest"

#define PWM_MAGIC_NUM 0xF1

#define PWM_SET			_IOW(PWM_MAGIC_NUM, 0, PWMset)
#define PWM_INT_ARRAY	_IOW(PWM_MAGIC_NUM, 1, int*)

typedef struct pwmset {

	int pin1, pin2, pin3, pin4;
	int freq, max, min;

} PWMset;

long pwm_ioctl(struct file *, unsigned int, unsigned long);
void pwm_task(void);
