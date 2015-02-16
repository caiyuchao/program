#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void foo(void){
	int i;
	uint32_t a = 0x01020304;
	char *x = (char *)&a;
	if(*x = 0x04)
		printf("little endian\n");
	else if(*x == 0x01)
		printf("big endian\n");
	else
		printf("error\n");
}

static void bar(void){
	char *str = "HTTP/0.9/1.0/1.1 200";
	char http[] = "HTTP";
	printf("0x%08x 0x%08x 0x%08x 0x%08x\n", 
			*(uint32_t *)str, 
			*((uint32_t *)str+1),
			*((uint32_t *)str+2),
			*((uint32_t *)str+3));
	printf("HTTP 0x%08x\n", *(uint32_t *)"HTTP"); 
}

int main(int argc, const char *argv[])
{
	foo();
	bar();
	
	return 0;
}
