#!/bin/bash
# only bash

for (( i=1; i<=10; i++ )); do 
	echo $(expr $i \* 4)
done;
