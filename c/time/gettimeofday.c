#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>

int main(int argc, const char *argv[])
{
	struct  timeval    tv;
	struct  timezone   tz;

	gettimeofday(&tv,&tz);
	printf("tv_sec:%ld\n",tv.tv_sec);
	printf("tv_usec:%ld\n",tv.tv_usec);
	printf("tz_minuteswest:%d\n",tz.tz_minuteswest);
	printf("tz_dsttime:%d\n",tz.tz_dsttime);

	struct  timeval start;
	struct  timeval end;
	unsigned  long diff;

	gettimeofday(&start,NULL);
	sleep(2);
	gettimeofday(&end,NULL);
	diff = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("thedifference is %ld\n",diff);
	return 0;
}
