#!/bin/sh


for i in $(seq 3); do 
	echo $i
done

for i in $(seq 3); do echo $i; done

for line in $(cat /etc/hosts); do
	echo $line
done



