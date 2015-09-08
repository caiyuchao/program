#define STR "heloo world"
#define STR1 "#STR" 
#ifndef STR2
//#error must have xxx
#endif

#define ALIAS_FILTER(a, b, c, ptr)  \
({                                  \
	ptr = (char *)a; \
	ptr[0] == b && ptr[1] == c ? &ptr[2] : &ptr[0]; \
})

// Return the offset of 'member' relative to the beginning of a struct type
STR
STR1
ALIAS_FILTER(e->isp, 'p', '_', ptr);
