ifneq ($(KERNELRELEASE),)

obj-m := iio_bpm_dma.o
obj-m += pdev_bpm_mmio.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all: modules

modules modules_install clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) $@

.PHONY: all modules modules_install clean

endif
