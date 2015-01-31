.PHONY: all clean

all: $(OBJS) $(EXAMPLES)

%: %.o
	$(CC) $< -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

%.s: %.c
	$(CC) $< $(CFLAGS) -S -o $@

%.e: %.c
	$(CC) $< $(CFLAGS) -E -o $@

clean:
	rm -rf $(EXAMPLES) $(OBJS)
