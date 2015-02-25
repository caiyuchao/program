#include "crash.h"

static void foo(void){
	*(int *)0 = 1;
}

static void bar(void){
	foo();
}

void crash(void)
{
	bar();
}

