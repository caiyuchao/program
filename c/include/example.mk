.PHONY: all clean

all: $(OBJS) $(EXAMPLES)

%: %.o
	$(CC) $< -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

%.s: %.c
	$(CC) $< -S -o $@ $(CFLAGS)

%.e: %.c
	$(CC) $< -E -o $@ $(CFLAGS)

clean:
	rm -rf $(EXAMPLES) $(OBJS)
