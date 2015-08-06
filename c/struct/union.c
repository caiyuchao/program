#include <stdio.h>
#include <stdint.h>

struct foo_t{
	union{
		uint32_t a;
		uint16_t b[2];
	};
};

struct bar_t{
	union{
		uint32_t a;
		uint16_t b[2];
	}b;
};


int main(){
	struct foo_t f;
	struct bar_t b;
	f.a = 0x01020304;
	b.b.a = 0x01020304;
	printf("%08x, %04x, %04x\n", f.a, f.b[0], f.b[1]);
	printf("%08x, %04x, %04x\n", b.b.a, b.b.b[0], b.b.b[1]);
	return 0;
}
