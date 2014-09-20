#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME		"my_char"

// my char device structure
struct mychar_dev{
	unsigned int count;
	struct cdev cdev;
}mychar;

static dev_t mychar_dev_number;
struct class *mychar_class;

int mychar_open(struct inode *inode, struct file *file){

	struct mychar_dev *mychar_devp;

	mychar_devp = container_of(inode->i_cdev, struct mychar_dev, cdev);

	file->private_data = mychar_devp;

	mychar_devp->count++;

	printk("mychar device opened %d time(s)\n", mychar_devp->count);
	return 0;
}

int mychar_close(struct inode *inode, struct file *file){
	
	printk("mychar device closed\n");
	return 0;
}

static struct file_operations mychar_ops = {
	.owner	=	THIS_MODULE,
	.open	=	mychar_open,
	.release =	mychar_close,
//	.read	= 	mychar_read,
//	.write	=	mychar_write,
};

int __init mychar_init(void){
	
	int ret;

	// get device numbers
	if(alloc_chrdev_region(&mychar_dev_number, 0, 1, DEVICE_NAME) < 0){
		printk(KERN_DEBUG "Error registering the device\n");
		return 1;
	}

	// create class
	mychar_class = class_create(THIS_MODULE, DEVICE_NAME);

	// connect the file operations with the cdev
	cdev_init(&mychar.cdev, &mychar_ops);
	mychar.cdev.owner	=	THIS_MODULE;

	// connect the major/minor number with the cdev
	ret = cdev_add(&mychar.cdev, mychar_dev_number, 1);
	if(ret){
		printk("cdev_add failed\n");
		return ret;
	}

	// send uevent to udev so it'll create /dev nodes
	device_create(mychar_class, NULL, mychar_dev_number, NULL, "mychar");

	mychar.count = 0;

	printk("mychar driver initialized [%d/%d]\n", MAJOR(mychar_dev_number), MINOR(mychar_dev_number));
	return 0;
}

void __exit mychar_exit(void){
	
	// release the major number
	unregister_chrdev_region(mychar_dev_number, 1);
	// destroy mychar device  (if you don't do this you will get a kernel Oops the second time you insmod the driver)
	device_destroy(mychar_class, mychar_dev_number);
	// destroy mychar_class
	class_destroy(mychar_class);

	printk("mychar driver removed\n");
}

module_init(mychar_init);
module_exit(mychar_exit);

MODULE_AUTHOR("Javi Jap");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("basic char driver for learning LDD");
