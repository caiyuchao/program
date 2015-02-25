#include <stdio.h>
#include <string.h>

void foo(char *p)
{
	int *crash = NULL;
	*crash = 1;
}
int main(int argc, char ** argv)
{
	char *p = "hello world";
	char *q = strdup("hello world");
	foo(p);
    return 0;

}
