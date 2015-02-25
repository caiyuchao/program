#include "crash.h"

void crash(void)
{
	*(int *)0 = 1;
}

