/**
 * Cyclictest-like latency measurement for Wiring Pi.
 * Copyright (c) 2017 Shao-Hua Wang.
 */

#include <alchemy/timer.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

#define INTERVAL (rt_timer_ticks2ns(end) - rt_timer_ticks2ns(start))
#define LATENCY (INTERVAL - test_period * 1000)
#define array_size 10000

/* get user-defined variables by getopt() */
static int times = 0;
static int test_period = 0;
static char *output = NULL;

/* latency test variable */
static int negative_num = 0;
static int overrun_num = 0;
static int latency[array_size] = {0};
static long long int max_latency = 0;
static long long int min_latency = LONG_MAX;

/* real-time time measure function varible by Xenomai alchemy library */
static RTIME start, end;

int main(int argc, char **argv)
{
	int get;

	wiringPiSetup();

	while ((get = getopt(argc, argv, "p:n:o:")) != -1) {
		switch (get) {
		case 'p':
			test_period = atoi(optarg);
			break;
		case 'n':
			times = atoi(optarg);
			break;
		case 'o':
			output = optarg;
			break;
		case '?':
			break;
		default:
			break;
		}
	}

	if (times == 0) {
		printf("Default execute times set to 1000\n");
		times = 1000;
	}

	if (test_period == 0) {
		printf("Default test period set to 10 us\n");
		test_period = 10;
	}

	printf("%d %d\n", times, test_period);

	for (int i = 0; i < times; i++) {
		start = rt_timer_read();
		delayMicroseconds(test_period);
		end = rt_timer_read();

		if (LATENCY / 1000 < 0)
			negative_num++;
		else if (LATENCY / 1000 > array_size)
			overrun_num++;
		else
			latency[LATENCY / 1000]++;

		if (LATENCY / 1000 < min_latency)
			min_latency = LATENCY / 1000;

		if (LATENCY / 1000 > max_latency)
			max_latency = LATENCY / 1000;
	}

	if (output != NULL) {
		FILE *out_fd = fopen(output, "w+");

		fprintf(out_fd, "# Max:%lld Min:%lld Overrun:%d Negative:%d\n",
			max_latency, min_latency, overrun_num, negative_num);

		for (int i = 0; i <= max_latency; i++) {
			fprintf(out_fd, "%d %d\n", i, latency[i]);
		}
	}

	return 0;
}
