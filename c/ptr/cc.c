#include <stdio.h>

void foo(int argc, void *data){
	char **args = (char **)data;
	int i;
	for(i = 0; i< argc; i++){
		printf("%d:%s\n", i, args[i]);
	}
}

int main(int argc, char **args){
	foo(argc, (void *)args);
	return 0;
}
