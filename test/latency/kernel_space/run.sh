#!bin/bash

sudo insmod kernel_latency.ko times=100000 test_period=10
