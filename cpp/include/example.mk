.PHONY: all clean

proj_deps := $(foreach n,$(objs),$(if $($(n:.o=-objs)),$($(n:.o=-objs)),$(n)))

proj_objs := $(subst .o,,$(objs))

all: $(proj_objs) $(OBJS) $(EXAMPLES) $(objs_e)

$(proj_objs): $(proj_deps)

$(OBJS):%: %.o
	$(CXX) $< -o $@ $(LDFLAGS)

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(proj_objs): %: 
	$(CXX) $(if $($(@)-objs), $($(@)-objs), $(@).o) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

%.s: %.cpp
	$(CXX) -S $(CFLAGS) $< -o $@

%.e: %.cpp
	$(CXX) -E $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXAMPLES) $(OBJS) $(proj_objs) $(proj_deps) $(addsuffix .o, $(OBJS))

test:
	echo $(proj_deps) $(proj_objs) $(OBJS) $(EXAMPLES) $(addsuffix .o, $(OBJS)) 

