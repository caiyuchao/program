# use pkg-config for getting CFLAGS abd LDFLAGS
LDFLAGS+= -L/usr/local/lib -lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -lswresample
CFLAGS= -I/usr/local/include -I.
EXAMPLES= mediainfo

OBJS=$(addsuffix .o,$(EXAMPLES))

%: %.o
	$(CC) $< $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

.phony: all clean

all: $(OBJS) $(EXAMPLES)

mediainfo: ffmpeg.o  cmdutils.o mediainfo.o ffprobe.o
	$(CC) ffmpeg.o  cmdutils.o ffprobe.o mediainfo.o $(LDFLAGS) -o $@

clean:
	rm -rf $(EXAMPLES) $(OBJS) *.o
