#!/bin/sh
./cortex -i ./core -f reg,cal
addr2line -e main `./cortex -i ./core -f reg,cal | grep -E '#[0-9]+ at 0x[0-9a-f]{8}' | awk '{print $3}'`
