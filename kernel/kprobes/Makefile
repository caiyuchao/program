all: kprobe_example.ko

kprobe_example.ko: kprobe_example.c
	make -C /lib/modules/`uname -r`/build M=$(shell pwd) modules

run:
	sudo rmmod kprobe_example
	sudo insmod ./kprobe_example.ko
