#include <stdio.h>
#include <stdlib.h>

struct s{
	int size;
	int i[];
};

void dump_s(struct s *s){
	int i;
	for(i = 0; i < s->size; i++){
		printf("%d:%d\n", i, s->i[i]);
	}
}
int main(int argc, const char *argv[])
{
	struct s *a;
	int i;
	a = calloc(1, sizeof(struct s) + argc * sizeof(int));
	a->size = argc;
	for(i=0; i<argc; i++){
		a->i[i] = atoi(argv[i]);
	}
	dump_s(a);
	return 0;
}
