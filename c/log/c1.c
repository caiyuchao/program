#define _DEBUG 7
#include "debug.h"


void bar(int a){
//	die(1, "hello world %d\n", 1);
	panic("hello world %d\n", 1);
}

void foo(int a){
	bar(2*a);
}

int main(){
	dlog("hello world %d\n", 1);
	elog("hello world %d\n", 1);
	foo(1);
	return 0;
}
