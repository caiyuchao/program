ifneq ($(KERNELRELEASE),)
	obj-m += hello-1.o
	obj-m += hello-2.o
	obj-m += hello-3.o
	obj-m += hello-4.o
	obj-m += hello-5.o
	obj-m += startstop.o
	startstop-objs := start.o stop.o
	obj-m += startstop.o
	obj-m += chardev.o
	obj-m += procfs.o
	obj-m += procfs-2.o
	obj-m += chardev-2.o
	obj-m += sleep.o
	obj-m += print_string.o
	obj-m += kbleds.o
	obj-m += kset-example.o
	#obj-m += sched.o
	#obj-m += intrpt.o
else

KERNEL_DIR = /lib/modules/`uname -r`/build
MODULEDIR := $(shell pwd)

.PHONY: modules
default: modules

modules:
	make -C $(KERNEL_DIR) M=$(MODULEDIR) modules

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko *.ko.unsigned
	rm -rf .tmp_versions
endif
