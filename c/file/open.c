#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(){
	int fd;
	ssize_t len;
	char buff[1024];

	fd = open("/etc/hosts", O_RDONLY);
	if(fd == -1 ){
		printf("open error\n");
		return 1;
	}

	while((len = read(fd, buff, 1024))) {
		buff[len] = '\0';
		printf("%s", buff);
	}

	close(fd);

	return 0;
}
