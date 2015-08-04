#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct block_t {
	char b[188];
};

struct foo_t{
	uint32_t a;
	uint32_t b;
	union {
		char d[0];
		struct block_t e[0];
	}c;
};


int main(){
	struct foo_t *foo;
	int i, j;

	printf("struct foo_t size: %lu\n", sizeof(struct foo_t));
	foo = (struct foo_t *)malloc(sizeof(struct foo_t) + 188*10);
	for(i = 0; i<10; i++){
		for(j=0; j<188; j++){
			foo->c.e[i].b[j] = i+j;
		}
	}
	for(i=0; i<1880; i++){
		printf("%02x ", (uint8_t)foo->c.d[i]);
		if(i%188 == 187) printf("\n");
	}
	return 0;
}
