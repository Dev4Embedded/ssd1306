# SPDX-License-Identifier: GPL-2.0
# Makefile for SSD1306 OLED display driver

obj-$(CONFIG_SSD1306) += ssd1306.o
ssd1306-$(CONFIG_SSD1306) := ssd1306-i2c.o \
			     ssd1306-drv.o \
			     ssd1306-font.o \
			     ssd1306-cmode.o

modules modules_install clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
help:
	@echo "Provide neccesary variables:"
	@echo "    KERNELDIR - path to kernel source"
	@echo "    CONFIG_SSD1306 - type of module"
	@echo "example:"
	@echo "    make KERNELDIR=\"/lib/modules/5.4.1/build\" CONFIG_SSD1306=m"
