#include <stdio.h>
#include <string.h>

static char *a[] = {
	"a",
	"b"
};

int main(){
	int i;
	char s[] = "hello world";
	printf("%*s\n", 5, s);
	printf("%.*s\n", 5, s);

	for(i=0; i< sizeof(a) / sizeof(*a); i++){
		printf("%d:%s\n", i, a[i]);
	}
	return 0;
}
