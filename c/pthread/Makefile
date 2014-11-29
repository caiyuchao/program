all: cond a

a:	a.c
	gcc -pthread -g -o a a.c
queue.o:	queue.h queue.c
	gcc -g -c queue.c
control.o:	control.h control.c
	gcc -g -c control.c

cond:	cond.c 
	gcc -lpthread -g -I. -o cond  cond.c
