#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

struct foo{
	uint32_t size;
	char buf[0];
};

struct bar{
	uint32_t size;
	char buf[0];
	char buffer[];
};


int main(int argc, const char *argv[])
{
	struct foo *f;
	struct bar *b;
	int ia[] = {1,2,3,4,5};
	int s;

	f = (struct foo *)malloc(sizeof(struct foo) + 100*sizeof(char));
	b = (struct bar *)malloc(sizeof(struct bar) + 100*sizeof(char));

	f->buf[0] = 'a';
	f->buf[1] = '\0';
	b->buf[0] = 'a';
	b->buf[1] = '\0';

	printf("sizeof(f)=%d\nsizeof(b)=%d\n", (int)sizeof(*f), (int)sizeof(*b));
	printf("f.buf:%s\n", f->buf);
	printf("b.buf:%s\n", b->buffer);
	printf("sizeof(ia):%d/%d\n",(int)sizeof(ia), (int)(sizeof(ia)/sizeof(ia[0])));


	srand((unsigned)time(NULL));
	s = rand()%100;
	{
		char a[s];
		printf("sizeof(a):%d\n",(int)sizeof(a));
	}

	return 0;
}
