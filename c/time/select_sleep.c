#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 100000;
	select(0, NULL, NULL, NULL, &tv);
	return 0;
}
