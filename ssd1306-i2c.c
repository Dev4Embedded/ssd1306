// SPDX-License-Identifier: (GPL-2.0 OR MIT)

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/i2c.h>

#include "ssd1306.h"

static LIST_HEAD(ssd1306_list);
static DEFINE_SPINLOCK(ssd1306_list_lock);

static dev_t             dev_number;
static struct class     *disp_class;
static struct device    *dev_oled;
static struct i2c_device_id ssd1306_id[] = {
	{DEVICE_NAME, 0},
	{ }
};
static int ssd1306_probe(struct i2c_client *, const struct i2c_device_id *);
static struct i2c_driver ssd1306_i2c = {
	.driver = {
		.name	= DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
	.probe = ssd1306_probe,
	.remove = NULL,
	.id_table = ssd1306_id,
};

/**
 * @brief
 *     Setup SSD1306 device.
 *
 * @param[IN] *oled      pointer to SSD1306 device
 * @param[IN] *client    pointer to I2C client
 *
 * @return returns zero or negative error
 */
static int ssd1306_setup(struct ssd1306 *oled, struct i2c_client *client)
{
	if (!client || !oled) {
		printk(KERN_ALERT "I2C client does not exist\n");
		return -EPERM;
	}

	oled->i2c_client=client;

	return 0;
}

/**
 * @brief
 *     Probe I2C OLED display
 *
 * @param[IN] *client    pointer to I2C client
 * @param[IN] *id        poitner to I2C device ID
 *
 * @return returns zero or negative error
 */
static int ssd1306_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct ssd1306 *oled;

	int err;

	if (!client || !id) {
		printk(KERN_ALERT "I2C client doesn't exist\n");
		return -EPERM;
	}

	oled = (struct ssd1306 *)kmalloc(sizeof(struct ssd1306), GFP_KERNEL);
	if (IS_ERR_OR_NULL(oled)) {
		printk(KERN_DEBUG "Cannot allocate memory for driver\n");
		return -ENOMEM;
	}

	ssd1306_setup(oled, client);

	spin_lock(&ssd1306_list_lock);
	list_add(&oled->list, &ssd1306_list);
	spin_unlock(&ssd1306_list_lock);

	dev_oled = device_create(disp_class, NULL, dev_number,
				NULL, DEVICE_NAME);

	if (IS_ERR(dev_oled)) {
		err = IS_ERR(dev_oled);
		printk(KERN_DEBUG "Cannot create oled device\n");
		goto err_cdev;
	}

	printk(KERN_DEBUG "Device %s created\n", DEVICE_NAME);

	err = ssd1306_init_hw(oled);
	if (err) {
		printk(KERN_DEBUG "SSD1306 device doesn't response\n");
		goto err_device;
	}

	printk(KERN_INFO "Driver successfully probed");

	return 0;

err_device:
	class_destroy(disp_class);
err_cdev:
	cdev_del(&oled->char_dev);

	spin_lock(&ssd1306_list_lock);
	list_del(&oled->list);
	spin_unlock(&ssd1306_list_lock);

	kfree(oled);

	return err;
}

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
	i2c_del_driver(&ssd1306_i2c);
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
