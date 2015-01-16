#include <stdio.h>
#include <stdlib.h>


void (*handler)(int i, char *str);

typedef void (*foo_handler)(int i, char *str);

struct {
	foo_handler handler;	
}ctx;

void bar(foo_handler handler){
	handler(3, "from bar");
}

static void foo(int i, char *str){
	printf("%d %s\n", i, str);
}

int main(int argc, const char *argv[])
{
	handler = foo;
	handler(1, "hello world");

	ctx.handler = foo;
	ctx.handler(2, "Hw");

	bar(foo);

	return 0;
}
