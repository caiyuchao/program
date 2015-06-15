# use pkg-config for getting CFLAGS abd LDFLAGS
LDFLAGS+= -L/usr/local/lib -lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -lswresample
EXAMPLES=decoding_encoding filtering metadata muxing thumbnail

OBJS=$(addsuffix .o,$(EXAMPLES))

%: %.o
	$(CC) $< $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

.phony: all clean

all: $(OBJS) $(EXAMPLES)

clean:
	rm -rf $(EXAMPLES) $(OBJS)
