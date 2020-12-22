# Linux driver for SSD1306 OLED display

![SSD1306 connected to stm32mp1-dk2 board](https://raw.githubusercontent.com/Dev4Embedded/ssd1306/master/img/ssd1306.png)

These source codes implementing Linux driver for **SSD1306** Organic-LED display.
In this phase of work resolution of the display is hard-coded and equal
128 x 32 pixels.

These module will have two main features:

Character device operations (mainly for write operation).

- Driver use default kernel font VGA8x8
- Add +1 pixel (by default) free space between characters what give 14
  character per line
- Theoretical maximum display capacity is 56 characters. In practice, 
  some new lines can be added, which will reduce capacity. 
  (Currently in development stage)

Send commands using ioctrl

- (Not implemented yet)

## How to build

You can add this repository to you kernel distribution. Just put it to:

```
<your-linux>/drivers
```

and modify Kconfig in the same directory adding this line:

```git
 source "drivers/counter/Kconfig"
 
+source "drivers/ssd1306/Kconfig"
+
 endmenu
```

After that, you should see SSD1306 module in menuconfig.

You can also built it as separate module:

```
make CONFIG_SSD1306=m KERNELDIR=<path-to-your-kernel-distribution>
```

## How to use

1. Inform the kernel about the device connected to I2C bus:

```sh
echo ssd1306 0x3C > /sys/class/i2c-adapter/i2c-<number>/new_device
```
2. Install the module 

```sh
insmod ssd1306.ko
```

3. Try to display some message on the display

```sh
echo "Hello World!" > /dev/ssd1306
```
