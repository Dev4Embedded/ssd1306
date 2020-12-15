// SPDX-License-Identifier: (GPL-2.0 OR MIT)

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/i2c.h>

#include "ssd1306.h"

static dev_t             dev_number;
static struct class     *disp_class;
static struct i2c_device_id ssd1306_id[] = {
	{DEVICE_NAME, 0},
	{ }
};
static struct i2c_driver ssd1306_i2c = {
	.driver = {
		.name	= DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
	.probe = NULL,
	.remove = NULL,
	.id_table = ssd1306_id,
};

/**
 * @brief
 *     Kernel module initialization function. Creates character device for
 *     ASCII printing. Add I2C driver for communication to display.
 *
 * @return returns zero or negative error
 */
static int __init ssd1306_init(void)
{
	int err;

	err = alloc_chrdev_region(&dev_number, MINOR_BASE,
				  MINOR_COUNT, DEVICE_NAME);

	if (err) {
		printk(KERN_ALERT "Cannot allocate a range of char device\n");
		goto err_alloc;
	}

	disp_class=class_create(THIS_MODULE, CLASS_NAME);

	if (IS_ERR(disp_class)) {
		err = IS_ERR(disp_class);
		printk(KERN_ALERT "Cannot create structure of class\n");
		goto err_class;
	}

	err = i2c_add_driver(&ssd1306_i2c);
	if (err) {
		printk(KERN_ALERT "Can't register I2C driver %s\n",
		       ssd1306_i2c.driver.name);
		goto err_i2c;
	}

	printk(KERN_DEBUG "SSD1306 driver initialization done\n");
	return 0;

err_i2c:
	class_destroy(disp_class);
err_class:
	unregister_chrdev_region(dev_number, MINOR_COUNT);
err_alloc:
	return err;
}

/**
 * @brief
 *     Delete all objects. Frees memory.
 */
static void __exit ssd1306_exit(void)
{
	class_destroy(disp_class);
	unregister_chrdev_region(dev_number, MINOR_COUNT);
	i2c_del_driver(&ssd1306_i2c);
	printk(KERN_DEBUG "SSD1306 driver successfully removed\n");
}

module_init(ssd1306_init);
module_exit(ssd1306_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stanislaw Pietrzak <integralzerox@gmail.com>");
MODULE_DESCRIPTION("SSD1306 OLED Display driver via I2C");
