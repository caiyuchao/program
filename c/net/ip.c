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

typedef uint32_t __be32;

static in_network(__be32 ip, __be32 network, __be32 netmask){
	if((ip & netmask) == (network & netmask))
		return 1;
	else
		return 0;
}
int main(int argc, char *argv[])
{
	struct in_addr a, a1, n;
	char *ip = "192.168.31.1";
	char *netmask = "255.255.254.0";

	if (argc != 2) {
		fprintf(stderr, "%s <dotted-address>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	//dest ip
	if (inet_aton(argv[1], &a1) == 0) {
		perror("inet_aton");
		exit(EXIT_FAILURE);
	}

	if (inet_aton(ip, &a) == 0) {
		perror("inet_aton");
		exit(EXIT_FAILURE);
	}

	if (inet_aton(netmask, &n) == 0) {
		perror("inet_aton");
		exit(EXIT_FAILURE);
	}

	printf("%08x %08x %08x\n", a1.s_addr, a.s_addr, n.s_addr);
	printf("%d\n", in_network(a1.s_addr, a.s_addr, n.s_addr));
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
