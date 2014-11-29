#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct node{
	char * str;
};


static int cmp_node(const void *p1, const void *p2)
{
	/* The actual arguments to this function are "pointers to
	   pointers to char", but strcmp(3) arguments are "pointers
	   to char", hence the following cast plus dereference */
	struct node *node1, *node2;
	node1 = *(struct node **)p1;
	node2 = *(struct node **)p2;


	return strcmp(node1->str, node2->str);
	//return strcmp(* (char * const *) p1, * (char * const *) p2);
}

int main(int argc, char *argv[])
{
	int i;
	struct node **data, *_data;
	int size;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <string>...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	size = argc - 1;


	_data = calloc(size, sizeof(struct node));
	data = calloc(size, sizeof(struct node *));

	for(i = 0; i < size; i++){
		_data[i].str = argv[i+1];
		data[i] = &_data[i];
	}

	qsort(&data[0], size, sizeof(data[0]), cmp_node);



	//qsort(&argv[1], argc - 1, sizeof(argv[1]), cmpstringp);

	for (i = 0; i < size; i++)
		puts(data[i]->str);

	exit(EXIT_SUCCESS);
}

