#include "25_cgo.h"

void hw(void){
	fprintf(stdout, "hello from c.hw()\n");
}

int64_t runtimeNano(void){
	struct timespec ts; 
    clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec*1000000000+ts.tv_nsec;
}
