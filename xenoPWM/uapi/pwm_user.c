/*
	Access RTDM device for software PWM from user space.
	Shao-Hua Wang
	2017.02.24
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "pwm_user.h"
#include "xenopwm.h"

void pinMode(int filp, int pin, int type) {

	int ret;
	int tmp[2];

	tmp[0] = pin;
	tmp[1] = type;

	ret = ioctl(filp, PIN_SET, tmp);
	if (ret < 0) {
		printf("ioctl for PIN_SET failed: %d %s\n", ret, strerror(errno));
	}

}

void digitalWrite(int filp, int pin, int status) {

	int ret;
	int tmp[2];

	tmp[0] = pin;
	tmp[1] = status;

	ret = ioctl(filp, GPIO_SET, tmp);
	if (ret < 0) {
		printf("ioctl for GPIO_SET failed: %d %s\n", ret, strerror(errno));
	}

}

void softPwmCreate(int filp, int pin, int min, int max) {

	int ret;
	int tmp[3];

	tmp[0] = pin;
	tmp[1] = min;
	tmp[2] = max;

	ret = ioctl(filp, PWM_INIT, tmp);
	if (ret < 0) {
		printf("ioctl for PWM_INIT failed: %d %s\n", ret, strerror(errno));
	}

}

void softPwmWrite(int filp, int pin, int duty_cycle) {

	int ret;
	int tmp[2];

	tmp[0] = pin;
	tmp[1] = duty_cycle;

	ret = ioctl(filp, PWM_SET, tmp);
	if (ret < 0) {
		printf("ioctl for PWM_SET failed: %d %s\n", ret, strerror(errno));
	}

}
