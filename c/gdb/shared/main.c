#include "crash.h"

void foo(void){
	crash();
}

void bar(void){
	foo();
}

int main(int argc, char ** argv)
{
	foo();
    return 0;
}
