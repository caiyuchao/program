#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct {
	uint32_t a;
	uint16_t b;
	uint64_t c;
} a_t;

int main(){
	a_t a, b, *p;
	a.a = 0x01020304;
	a.b = 0x0102;
	a.c = 0x0102030405060708;
	p = &a;
	//b = p[0];
	b = *p;
	printf("%x %x %lx\n", a.a, a.b, a.c);
	printf("%x %x %lx\n", b.a, b.b, b.c);


}
