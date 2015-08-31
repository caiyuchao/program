#include <stdlib.h>
#include <stdio.h>

int main(){
	char *p, ps1[] = "[\\u@\\h:\\w]\\$\\$";

	p = getenv("PS1");
	printf("ps1 :%s\n", p);

	setenv("PS1", ps1, 1);

	p = getenv("PS1");
	printf("ps1 :%s\n", p);

	return 0;
}
