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
static struct device    *dev_oled;
static struct i2c_device_id ssd1306_id[] = {
	{DEVICE_NAME, 0},
	{ }
};
static int ssd1306_probe(struct i2c_client *, const struct i2c_device_id *);
static int ssd1306_remove(struct i2c_client *);
static struct i2c_driver ssd1306_i2c = {
	.driver = {
		.name	= DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
	.probe = ssd1306_probe,
	.remove = ssd1306_remove,
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
		LOG(KERN_ALERT, "I2C client does not exist");
		return -EPERM;
	}

	oled->i2c_client = client;

	oled->disp_buff = (uint8_t*)kmalloc(DISP_BUFF_SIZE, GFP_KERNEL);
	if (!oled->disp_buff)
		return -ENOMEM;

	memset(oled->disp_buff, 0, DISP_BUFF_SIZE);

	//Inform the driver about data stream:
	oled->disp_buff[0] = (uint8_t)SET_DISP_START_LINE;

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
		LOG(KERN_ALERT, "I2C client doesn't exist");
		return -EPERM;
	}

	oled = (struct ssd1306 *)kmalloc(sizeof(struct ssd1306), GFP_KERNEL);
	if (IS_ERR_OR_NULL(oled)) {
		LOG(KERN_DEBUG, "Cannot allocate memory for driver");
		return -ENOMEM;
	}

	err = ssd1306_setup(oled, client);
	if (err) {
		LOG(KERN_DEBUG, "Cannot setup OLED display");
		goto err_malloc;
	}

	dev_oled = device_create(disp_class, NULL, dev_number,
				NULL, DEVICE_NAME);

	if (IS_ERR(dev_oled)) {
		err = IS_ERR(dev_oled);
		LOG(KERN_DEBUG, "Cannot create oled device");
		goto err_cdev;
	}

	i2c_set_clientdata(client, oled);

	LOG(KERN_DEBUG, "Device %s created", DEVICE_NAME);

	err = ssd1306_init_hw(oled);
	if (err) {
		LOG(KERN_DEBUG, "SSD1306 device doesn't response");
		goto err_device;
	}

	LOG(KERN_DEBUG, "Driver successfully probed");

	return 0;

err_device:
	class_destroy(disp_class);
err_cdev:
	cdev_del(&oled->char_dev);
err_malloc:
	kfree(oled);

	return err;
}

static int ssd1306_remove(struct i2c_client *client)
{
	struct ssd1306* oled;

	if (!client) {
		LOG(KERN_ALERT, "I2C client device does not exist");
		return -ENXIO;
	}

	oled = i2c_get_clientdata(client);

	if (IS_ERR_OR_NULL(oled)) {
		LOG(KERN_ALERT, "I2C does not assigned to the display");
		return -ENXIO;
	}

	(void)ssd1306_deinit_hw(oled);

	kfree(oled->disp_buff);

	LOG(KERN_DEBUG, "I2C bus driver for display removed");

	return 0;
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
		LOG(KERN_ALERT, "Cannot allocate a range of char device");
		goto err_alloc;
	}

	disp_class=class_create(THIS_MODULE, CLASS_NAME);

	if (IS_ERR(disp_class)) {
		err = IS_ERR(disp_class);
		LOG(KERN_ALERT, "Cannot create structure of class");
		goto err_class;
	}

	err = i2c_add_driver(&ssd1306_i2c);
	if (err) {
		LOG(KERN_ALERT, "Can't register I2C driver %s",
		       ssd1306_i2c.driver.name);
		goto err_i2c;
	}

	LOG(KERN_DEBUG, "SSD1306 driver initialization done");
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
	device_destroy(disp_class, dev_number);
	class_destroy(disp_class);
	unregister_chrdev_region(dev_number, MINOR_COUNT);

	LOG(KERN_DEBUG, "SSD1306 driver successfully removed");
}

module_init(ssd1306_init);
module_exit(ssd1306_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stanislaw Pietrzak <integralzerox@gmail.com>");
MODULE_DESCRIPTION("SSD1306 OLED Display driver via I2C");
