#!/bin/sh
./cortex -i ./core -f reg,cal
addr2line -e a.debug 0x0000000000400501 0x000000000040052c
