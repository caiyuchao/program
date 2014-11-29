#include <stdio.h>
#include <stdlib.h>


int main(int argc, const char *argv[])
{
	int index = 2;
	int size = 5;
	int i;

	for (i = 0; i < size; i++){
		printf("%d %d\n", i, (index + size - i) % size);
		printf("%d %d\n", i, (index - i) % size);
	}
	return 0;
}



