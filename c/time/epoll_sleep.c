#include <sys/epoll.h>
#include <stdio.h>

int main(){
	int fd;
	fd = epoll_create(1);
	epoll_wait(fd, NULL, 1, 1100); //milliseconds
	return 0;
}
