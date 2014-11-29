#include <stdio.h>

void foo(char *p)
{
	int *crash = NULL;
	*crash = 1;
}
int main(int argc, char ** argv)
{
	char *p = "hello world";
	foo(p);
    return 0;

}
