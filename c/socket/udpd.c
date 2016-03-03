#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include<string.h>

#define ERR_EXIT(m) \
	do { \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while (0)

void echo_ser(int sock)
{
	char recvbuf[1024] = {0};
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int n;

	while (1)
	{

		peerlen = sizeof(peeraddr);
		memset(recvbuf, 0, sizeof(recvbuf));
		n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
				(struct sockaddr *)&peeraddr, &peerlen);
		if (n == -1)
		{

			if (errno == EINTR)
				continue;

			ERR_EXIT("recvfrom error");
		}
		else if(n > 0)
		{
			fputs(recvbuf, stdout);
			printf("recv %s:%d \n", inet_ntoa(peeraddr.sin_addr), peeraddr.sin_port);
			sendto(sock, recvbuf, n, 0,
					(struct sockaddr *)&peeraddr, peerlen);
		}
	}
	close(sock);
}

int main(int argc, char *argv[])
{
	int sock;

	if (argc != 2) {
		fprintf(stderr, "Usage\n\t%s <port> \n",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		ERR_EXIT("socket error");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("serv %s:%d \n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

	if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind error");

	echo_ser(sock);

	return 0;
}
