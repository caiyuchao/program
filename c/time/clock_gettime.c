#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	struct  timeval    tv;
	struct  timezone   tz;
	struct  timespec   ts;

	gettimeofday(&tv,&tz);
	printf("tv_sec:%lu\n",tv.tv_sec);
	printf("tv_usec:%lu\n",tv.tv_usec);
	printf("tz_minuteswest:%u\n",tz.tz_minuteswest);
	printf("tz_dsttime:%u\n",tz.tz_dsttime);

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("tv_sec:%lu\n",ts.tv_sec);
	printf("tv_usec:%lu\n",ts.tv_nsec / 1000);

	return 0;
}
