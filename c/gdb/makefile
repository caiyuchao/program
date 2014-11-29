all: a a.debug

a: a.c
	gcc -g -o a a.c

a.debug: a
	objcopy --only-keep-debug a a.debug
	strip -g a

