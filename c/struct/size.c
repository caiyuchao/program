#include <stdio.h>
#include <stdint.h>

struct foo_t{
	uint32_t a;
	uint32_t b;
	char c[0];
};


int main(){
	printf("struct foo_t size: %lu\n", sizeof(struct foo_t));
	return 0;
}
