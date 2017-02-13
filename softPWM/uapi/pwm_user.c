/*
	Access char device for software PWM from user space.
	Shao-Hua, Wang
	2017.02.11
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "pwm_user.h"
#include "softpwm.h"

int filp;

int main() {

	filp = open("softpwm", 0);
	if(filp < 0) {
		printf("failed to open device softpwm\n");
	}

	pinMode(21, INPUT);
	pinMode(21, OUTPUT);

	digitalWrite(21, HIGH);

	/*softPwmCreate(16, 0, 100);
	softPwmCreate(21, 0, 10000);

	sleep(1);

	softPwmWrite(16, 50);
	softPwmWrite(21, 1000);*/

	return 0;
}

void pinMode(int pin,int type) {

	int ret;
	int tmp[2];

	tmp[0] = pin;
	tmp[1] = type;

	printf("%d\n", pin);

	ret = ioctl(filp, PIN_SET, tmp);
	if (ret < 0) {
		printf("ioctl for PIN_SET failed: %d %s\n", ret, strerror(errno));
	}

}

void digitalWrite(int pin, int status) {

	int ret;
	int tmp[2];

	tmp[0] = pin;
	tmp[1] = status;

	ret = ioctl(filp, GPIO_SET, tmp);
	if (ret < 0) {
		printf("ioctl for GPIO_SET failed: %d %s\n", ret, strerror(errno));
	}

}

void softPwmCreate(int pin, int min, int max) {

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

void softPwmWrite(int pin, int duty_cycle) {

	int ret;
	int tmp[2];

	tmp[0] = pin;
	tmp[1] = duty_cycle;

	ret = ioctl(filp, PWM_SET, tmp);
	if (ret < 0) {
		printf("ioctl for PWM_SET failed: %d %s\n", ret, strerror(errno));
	}

}
