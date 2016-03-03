#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
do { \
	perror(m); \
	exit(EXIT_FAILURE); \
} while(0)

void echo_cli(char *addr, char *port)
{
	int ret, sock;
	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	struct sockaddr_in servaddr;

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	servaddr.sin_addr.s_addr = inet_addr(addr);

	printf("serv %s:%d \n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));


	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		ERR_EXIT("socket");

	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{

		sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

		ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
			ERR_EXIT("recvfrom");
		}

		fputs(recvbuf, stdout);
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(sock);


}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage\n\t%s <host> <port> \n",
				argv[0]);
		exit(EXIT_FAILURE);
	}


	echo_cli(argv[1], argv[2]);

	return 0;
}
