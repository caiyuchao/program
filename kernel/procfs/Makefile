KERNEL_DIR = /lib/modules/`uname -r`/build
MODULEDIR := $(shell pwd)

.PHONY: modules start stop restart
default: modules

modules:
	make -C $(KERNEL_DIR) M=$(MODULEDIR) modules

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko *.ko.unsigned
	rm -rf .tmp_versions
	rm -f procfs *.order *.symvers .*.cmd

start:
	insmod ./procfs.ko

stop:
	rmmod procfs

restart:
	rmmod procfs
	insmod ./procfs.ko

