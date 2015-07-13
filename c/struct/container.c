#include <stdio.h>
#include <stdint.h>

#ifndef offsetof
#define    offsetof(t,m)   (uintptr_t)((&((t *)0L)->m))
#endif 

#ifndef container_of
#define container_of(ptr, type, member)                 \
    ({                              \
        const typeof(((type *) NULL)->member) *__mptr = (ptr);  \
        (type *) ((char *) __mptr - offsetof(type, member));    \
    })
#endif

typedef struct foo_t{
	int a;
	int b;
}foo_t;

int main(){
	foo_t f;
	printf("f:%lx, f.a:%lx, f.b:%lx, conntainer_of(&f.b,foo_t,b):%lx\n", (uintptr_t)&f, (uintptr_t)&f.a, (uintptr_t)&f.b, (uintptr_t)container_of(&f.b, foo_t, b));
}
