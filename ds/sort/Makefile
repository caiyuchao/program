objects = MergeSort HeapSort HeapSort2

CFLAGS = -g -std=c99

LDFLAGS = 

all: $(objects)

%: %.c
	    gcc  $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	    rm -rf $(objects) *.dSYM
