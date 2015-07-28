.PHONY: all clean

proj_deps := $(foreach n,$(objs),$(if $($(n:.o=-objs)),$($(n:.o=-objs)),$(n)))

proj_objs := $(subst .o,,$(objs))

all: $(proj_deps) $(proj_objs) $(OBJS) $(EXAMPLES) 

$(proj_objs): $(proj_deps)

$(OBJS): $(addsuffix .o,$(OBJS))

%: 
	$(CC) $(if $($(@)-objs), $($(@)-objs), $(@).o) -o $@ $(LDFLAGS)

.o.c:
	$(CC) $(CFLAGS) $< -c -o $@

%.s: %.c
	$(CC) $(CFLAGS) $< -S -o $@

%.e: %.c
	$(CC) $(CFLAGS) $< -E -o $@

clean:
	rm -rf $(EXAMPLES) $(OBJS) $(proj_objs) $(proj_deps)

test:
	echo $(proj_deps) $(proj_objs) $(OBJS) $(EXAMPLES) 

