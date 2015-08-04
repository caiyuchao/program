#include <stdlib.h>
#include <mcheck.h>

int main(void) { 

	int* a = NULL;
	setenv("MALLOC_TRACE", "malloc_output.txt", 1);

	mtrace(); /* Starts the recording of memory allocations and releases */

	a = malloc(sizeof(int)); /* allocate memory and assign it to the pointer */
	if (a == NULL) {
		return 1; /* error */
	}

	//free(a); /* we free the memory we allocated so we don't have leaks */
	muntrace();

	return 0; /* exit */

}
