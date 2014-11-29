all: key cond_wait

CFLAGS= -lpthread

.c.o:
	gcc -c $?

key: key.o
	gcc key.o $(CFLAGS) -o key

cond_wait: cond_wait.o
	gcc cond_wait.o $(CFLAGS) -o cond_wait


clean:
	rm -f *.o key cond_wait


