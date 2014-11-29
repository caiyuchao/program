
.PHONY: subdirs $(SUBDIRS) clean


subdirs: $(SUBDIRS)

clean_dirs := $(addprefix _clean_,$(SUBDIRS))

KERNEL_DIR = /lib/modules/`uname -r`/build
MODULEDIR := $(shell pwd)

$(SUBDIRS):
	make -C $(KERNEL_DIR) M=$(MODULEDIR)/$@ modules

$(clean_dirs):
	make -C $(KERNEL_DIR) M=$(MODULEDIR)/$(patsubst_clean_%,%,$@) clean

clean: $(clean_dirs)
