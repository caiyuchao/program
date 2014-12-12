#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

char *trafficd_ctime(time_t time){
	static char buff[20];
	struct tm *tm;
	tm = localtime(&time);
	snprintf(buff, 20, "%4d-%02d-%02d %02d:%02d:%02d",
				tm->tm_year + 1900, tm->tm_mon, tm->tm_mday,
				tm->tm_hour, tm->tm_min, tm->tm_sec);
	return buff;
}

int main(int argc, const char *argv[])
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("%s %s\n", ctime(&tv.tv_sec), trafficd_ctime(1413378269));
	return 0;
}
