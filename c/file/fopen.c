#include <stdio.h>

int main(){
	FILE *fp;
	size_t len;
	char buff[1024];

	fp = fopen("/etc/hosts", "r");
	if(!fp){
		printf("fopen error\n");
		return 1;
	}

	while((len = fread(buff, 1, 1024, fp))) {
		buff[len] = '\0';
		printf("%s", buff);
	}

	fclose(fp);

	return 0;
}
