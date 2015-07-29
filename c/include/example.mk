.PHONY: all clean

proj_deps := $(foreach n,$(objs),$(if $($(n:.o=-objs)),$($(n:.o=-objs)),$(n)))

proj_objs := $(subst .o,,$(objs))

all: $(proj_objs) $(OBJS) $(EXAMPLES) 

$(proj_objs): $(proj_deps)

$(OBJS):%: %.o
	$(CC) $< -o $@ $(LDFLAGS)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(proj_objs): %: 
	$(CC) $(if $($(@)-objs), $($(@)-objs), $(@).o) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.s: %.c
	$(CC) -S $(CFLAGS) $< -o $@

%.e: %.c
	$(CC) -E $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXAMPLES) $(OBJS) $(proj_objs) $(proj_deps) $(addsuffix .o, $(OBJS))

test:
	echo $(proj_deps) $(proj_objs) $(OBJS) $(EXAMPLES) $(addsuffix .o, $(OBJS)) 

