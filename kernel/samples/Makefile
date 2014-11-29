SUBDIRS = \
hidraw \
hw_breakpoint \
kdb \
kfifo \
kobject \
kprobes \
rpmsg \
seccomp \
trace_events \
uhid


.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

KERNEL_DIR = /lib/modules/`uname -r`/build
MODULEDIR := $(shell pwd)

$(SUBDIRS):
	make -C $(KERNEL_DIR) M=$(MODULEDIR)/$@ modules

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko *.ko.unsigned
	rm -rf .tmp_versions