KERNEL_DIR = /lib/modules/`uname -r`/build
MODULEDIR := $(shell pwd)
PROG = http_stat

.PHONY: modules start stop restart
default: modules

modules:
	make -C $(KERNEL_DIR) M=$(MODULEDIR) modules

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko *.ko.unsigned
	rm -rf .tmp_versions
	rm -f hello *.order *.symvers .*.cmd

start:
	insmod ./$(PROG).ko

stop:
	rmmod $(PROG)

restart:
	insmod ./$(PROG).ko
	rmmod $(PROG)
