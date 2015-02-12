KERNEL_DIR = /lib/modules/`uname -r`/build
MODULEDIR := $(shell pwd)

.PHONY: modules start stop restart
default: modules

modules:
	make -C $(KERNEL_DIR) M=$(MODULEDIR) modules

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko *.ko.unsigned
	rm -rf .tmp_versions
	rm -f hello *.order *.symvers .*.cmd

start:
	insmod ./khello.ko

stop:
	rmmod khello

restart:
	insmod ./khello.ko
	rmmod khello
