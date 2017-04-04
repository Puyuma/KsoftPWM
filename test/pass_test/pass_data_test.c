/*
	Char device for software PWM in rapberry pi3 b+.
	Shao-Hua Wang
	2017.02.02
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>		//for kzalloc, kfree
#include <linux/kthread.h>	//for kthread_create, kthread_stop
#include <linux/uaccess.h>	//for access_ok

#include "pass_data_test.h"

struct cdev *cdevp = NULL;
PWMset *pwm_data;
int* a;
int dev_major;
int dev_minor = 0;

long pwm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {

	int err = 0;
	PWMset *data_tmp;
	int *tmp;
    
	if (_IOC_TYPE(cmd) != PWM_MAGIC_NUM) return -ENOTTY;
	if (_IOC_NR(cmd) > 1) return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	switch(cmd) {

		case PWM_SET:

			/*data_tmp = kzalloc(sizeof(PWMset), GFP_KERNEL); //GFP_KERNEL to allocate the normal memory
			if(!(pwm_data)) {
				printk(KERN_WARNING "failed to allocate data_tmp memory by kzalloc\n");
			}*/

			data_tmp = (PWMset *)arg;
			pwm_data->pin1 = data_tmp->pin1;
			pwm_data->pin2 = data_tmp->pin2;
			pwm_data->pin3 = data_tmp->pin3;
			pwm_data->pin4 = data_tmp->pin4;

			printk(KERN_ALERT "%d, %d, %d, %d\n", pwm_data->pin1, pwm_data->pin2, pwm_data->pin3, pwm_data->pin4);

			break;

		case PWM_INT_ARRAY:

			/*tmp = kzalloc(sizeof(int) * 2, GFP_KERNEL);
			if(!(tmp)) {
				printk(KERN_WARNING "failed to allocate tmp memory by kzalloc\n");
			}*/

			tmp = (int *)arg;
			a = tmp;

			printk(KERN_ALERT "%d, %d\n", a[0], a[1]);
			break;

	}

	return err;

}

struct file_operations pwm_fops = {
    .owner = THIS_MODULE,
	.unlocked_ioctl = pwm_ioctl,
};

int __init pwm_init(void) {

	int result;
	dev_t dev = 0;

	printk(KERN_INFO "SoftPWM module load.\n");

	//initial the PWM needed data
	pwm_data = kzalloc(sizeof(PWMset), GFP_KERNEL); //GFP_KERNEL to allocate the normal memory
	if(!(pwm_data)) {
		printk(KERN_WARNING "failed to allocate pwm_data memory by kzalloc\n");
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

	dev_t dev = MKDEV(dev_major, dev_minor);

	printk(KERN_INFO "SoftPWM module unload.\n");

	unregister_chrdev_region(dev, 1);

	kfree(cdevp);

}

MODULE_LICENSE("GPL");

module_init(pwm_init);
module_exit(pwm_exit);
