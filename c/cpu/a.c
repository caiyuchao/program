#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void foo(){
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

int main(int argc, const char *argv[])
{
	foo();
	
	return 0;
}
