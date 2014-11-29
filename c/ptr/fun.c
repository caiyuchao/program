#include <stdio.h>
#include <stdlib.h>

static void foo(int i, char *str){
	printf("%d %s\n", i, str);
}


void (*hook)(int i, char *str);



int main(int argc, const char *argv[])
{
	hook = foo;
	hook(1, "hello world");
	return 0;
}
