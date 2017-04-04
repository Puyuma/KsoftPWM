/**
 * Access RTDM device for software PWM from user space.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "pwm_user.h"
#include "xenopwm.h"

#define PIN1 16
#define PIN2 21

int filp;

int main()
{
	filp = open("/dev/rtdm/xenoPWM", 0);
	if (filp < 0) {
		printf("failed to open device xenoPWM : %s\n", strerror(errno));
	}

	pinMode(filp, PIN1, OUTPUT);
	pinMode(filp, PIN2, OUTPUT);

	softPwmCreate(filp, PIN1, 0, 100);
	softPwmCreate(filp, PIN2, 0, 10000);

	softPwmWrite(filp, PIN1, 50);
	softPwmWrite(filp, PIN2, 1000);

	return 0;
}
