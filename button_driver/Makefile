# Button driver Makefile

obj-m := button_driver.o

KDIR := /home/hung/Chloe/BBB/bb-kernel/KERNEL
CROSS_COMPILE := /home/hung/Chloe/BBB/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ARCH := arm
EXTRA_CFLAGS := -Wall -Wno-error
PWD := $(shell pwd)

all: ; $(MAKE) -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
clean: ; $(MAKE) -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean