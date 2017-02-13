/*
	Access char device for software PWM from user space.
	Shao-Hua, Wang
	2017.02.11
*/

#define LOW		0
#define HIGH	1
#define INPUT	0
#define OUTPUT	1

void pinMode(int pin,int status);
void digitalWrite(int pin, int status);
void softPwmCreate(int pin, int min, int max);
void softPwmWrite(int pin, int duty_cycle);
