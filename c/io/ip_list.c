#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FILE_NAME		"ip_list.txt"

#define DPRINTF(format, ...) fprintf(stderr, "%s(%d): " format, __func__, __LINE__, ## __VA_ARGS__)
#define D(format, ...) do { \
	DPRINTF(format, ##__VA_ARGS__); \
} while (0)


int main(int argc, const char *argv[])
{
	FILE *fp;
	char line[200];
	char network[16];
	int prefix, num;

	/* Open the PROCps kernel table. */
	if ((fp = fopen(FILE_NAME, "r")) == NULL) {
		D("fopen %s error\n", FILE_NAME);
		return;
	}

	while (fgets(line, sizeof(line), fp)) {
		num = sscanf(line, "%[^/\n]/%d", network, &prefix);
		if(num == 1){
			D("num:%d, network:%s/32\n", num, network);

		}else if(num == 2){
			D("num:%d, network:%s/%d\n", num, network, prefix);
		}
	}
	fclose(fp);

	return 0;
}
