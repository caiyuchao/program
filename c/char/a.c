#include <stdio.h>
#include <string.h>
int main(int argc, const char *argv[])
{
	char *br_ports[1024];
	char ifname[18];

	unsigned int i;
	for(i=0; i<1024; i++){
		br_ports[i] = NULL;
	}
	br_ports[3] = "hello world!";
	br_ports[30] = "hello world!";
	br_ports[300] = "hello world!";
	for(i=0; i<1024; i++){
		if(br_ports[i])
			printf("%d %s %d %d\n", i, br_ports[i], 
					strcmp(br_ports[300], br_ports[3]), strcmp(br_ports[30], br_ports[3]));
	}
	ifname[0] = '\0';
	printf("ifname:%s\n", ifname);
	printf("strlen(NULL)%d\n", strlen(""));
	i = 0x00646c72;
    printf("H%x Wo%s\n", 57616, &i);
    printf("%d %d\n", 2 );

	sprintf(ifname, "hello world");
	printf("%s\n", ifname);

	sprintf(ifname, "hello world");
	printf("%s\n", ifname);
	return 0;
}
