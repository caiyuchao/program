#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "md5.h"

int main(){
	md5_ctx_t ctx;
	uint32_t md5[4];
	char data[] = "hello world!";

	md5_begin(&ctx);
	md5_hash(data, strlen(data), &ctx);
	md5_end(md5, &ctx);
	printf("md5sum:%08x%08x%08x%08x\n", 
			htonl(md5[0]), htonl(md5[1]),
			htonl(md5[2]), htonl(md5[3]));
	return 0;
}
