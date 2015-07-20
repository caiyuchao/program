#include <stdio.h>
#include <string.h>

int main(){
	char s[] = "hello world";
	printf("%*s\n", 5, s);
	printf("%.*s\n", 5, s);
	return 0;
}
