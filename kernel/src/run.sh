#!/bin/sh
make -C /usr/src/linux/ SUBDIRS=$PWD modules V=1
gcc -o ioctl ioctl.c
gcc -o cat_noblock cat_noblock.c
