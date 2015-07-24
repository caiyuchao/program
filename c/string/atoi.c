#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(){
	char *str = "HTTP/1.1 200 OK\nxxooxxoo:xxooxox";
	uint32_t *resp = (uint32_t *)str;
	printf("%d\n", atoi((char *)&resp[2]));
	return 0;
}
