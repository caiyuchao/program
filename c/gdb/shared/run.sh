#!/bin/sh
cortex -i ./core -f reg,cal
addr2line -e main 0x00000000ba45b67e 0x00000000004006a1
