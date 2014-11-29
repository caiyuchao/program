#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#ifndef  NIPQUAD
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
((unsigned char *)&addr)[1], \
((unsigned char *)&addr)[2], \
((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"
#endif

#ifndef  HIPQUAD
#define HIPQUAD(addr) \
	((unsigned char *)&addr)[3], \
((unsigned char *)&addr)[2], \
((unsigned char *)&addr)[1], \
((unsigned char *)&addr)[0]
#define HIPQUAD_FMT "%u.%u.%u.%u"
#endif

#define _BSD_SOURCE
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	struct in_addr addr;

	if (argc != 2) {
		fprintf(stderr, "%s <dotted-address>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (inet_aton(argv[1], &addr) == 0) {
		perror("inet_aton");
		exit(EXIT_FAILURE);
	}

	printf("%s\n", inet_ntoa(addr));
	exit(EXIT_SUCCESS);
}


#if 0
int main(void)
{
	char buf[16];
	uint32_t s = 0xc0a81f64;
	inet_ntop(AF_INET, &s, buf, sizeof(buf));
	printf("address: %s\n", buf);
	printf("address: %u.%u.%u.%u\n", HIPQUAD(s));
	return 0;

}

#endif
