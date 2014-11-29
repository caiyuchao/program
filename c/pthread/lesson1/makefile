# use pkg-config for getting CFLAGS abd LDFLAGS
LDFLAGS= -lpthread
CFLAGS= -I/usr/local/include 
EXAMPLES= thread1 thread2 thread3 thread4 thread5

OBJS=$(addsuffix .o,$(EXAMPLES))

%: %.o
	$(CC) $< $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

.phony: all clean

all: $(OBJS) $(EXAMPLES)


clean:
	rm -rf $(EXAMPLES) $(OBJS)
