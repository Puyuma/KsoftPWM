/*
	Char device for software PWM in rapberry pi3 b+.
	Shao-Hua Wang
	2017.02.02
*/
#ifndef _SOFTPWM_H_
#define _SOFTPWM_H_

#include <linux/ioctl.h>

#define DEVICE_NAME "softPWM"
#define DEVICE_ACCESS_NAME "softpwm"

#define PWM_MAGIC_NUM 0xF1

#define PIN_SET		_IOW(PWM_MAGIC_NUM, 0, int*)
#define GPIO_SET	_IOW(PWM_MAGIC_NUM, 1, int*)
#define PWM_INIT	_IOW(PWM_MAGIC_NUM, 2, int*)
#define PWM_SET		_IOW(PWM_MAGIC_NUM, 3, int*)

#define PWM_FREQ 100

#define MAX_PWM_NUM			5

typedef struct duty_cycle {

	int pin;
	int duty_cycle;
	int min;
	int max;

} Dutycycle;

long pwm_ioctl(struct file *, unsigned int, unsigned long);
int pwm_task(void*);

#endif
