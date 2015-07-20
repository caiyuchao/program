#include <string.h>
#include <stdio.h>


int main(){
	char buff[4];
	char *str="0123456";
	strncpy(buff, str, sizeof(buff));
	printf("[%s] sizeof:%lu\n", buff, sizeof(buff));
}
