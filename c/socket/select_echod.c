#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

static int forward_port;

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

typedef struct {    /* one Client struct per connected client */
  int   fd;         /* fd, or -1 if available */
  struct sockaddr_in addr;
} Client;


Client   *client;        /* ptr to malloc'ed array */
int       client_size;   /* # entries in client[] array */


#define NALLOC  10      /* # client structs to alloc/realloc for */
#define	MAXLINE	4096			/* max line length */

static void
client_alloc(void)      /* alloc more entries in the client[] array */
{
    int     i;

    if (client == NULL)
        client = malloc(NALLOC * sizeof(Client));
    else
        client = realloc(client, (client_size+NALLOC)*sizeof(Client));
    if (client == NULL)
        fprintf(stderr, "can't alloc for client array error \n");

    /* initialize the new entries */
    for (i = client_size; i < client_size + NALLOC; i++)
        client[i].fd = -1;  /* fd of -1 means entry available */

    client_size += NALLOC;
}

/*
 * Called by loop() when connection request from a new client arrives.
 */
int
client_add(int fd, struct sockaddr_in *addr)
{
    int     i;

    if (client == NULL)     /* first time we're called */
        client_alloc();
again:
    for (i = 0; i < client_size; i++) {
        if (client[i].fd == -1) {   /* find an available entry */
            fprintf(stdout, "new connection: client_address %s, fd %d\n", 
                inet_ntoa(addr->sin_addr), fd);  
            client[i].fd = fd;
			memcpy(&client[i].addr, addr, sizeof(*addr));
            return(i);  /* return index in client[] array */
        }
    }

    /* client array full, time to realloc for more */
    client_alloc();
    goto again;     /* and search again (will work this time) */
}


/*
 * Called by loop() when we're done with a client.
 */
void
client_del(int fd)
{
    int     i;

    for (i = 0; i < client_size; i++) {
        if (client[i].fd == fd) {
            fprintf(stdout, "closed: client_address %s, fd %d\n",
              inet_ntoa(client[i].addr.sin_addr), client[i].fd);   
            client[i].fd = -1;
            return;
        }
    }
    fprintf(stderr, "can't find client entry for fd %d error \n", fd);
}


	static int
listen_socket(int listen_port)
{
	struct sockaddr_in a;
	int s;
	int yes;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket error \n");
		return -1;
	}
	yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
				(char *) &yes, sizeof(yes)) == -1) {
		fprintf(stderr, "setsockopt error \n");
		close(s);
		return -1;
	}
	memset(&a, 0, sizeof(a));
	a.sin_port = htons(listen_port);
	a.sin_family = AF_INET;
	if (bind(s, (struct sockaddr *) &a, sizeof(a)) == -1) {
		fprintf(stderr, "bind error\n");
		close(s);
		return -1;
	}
	printf("accepting connections on port %d\n", listen_port);
	listen(s, 10);
	return s;
}

	static int
connect_socket(int connect_port, char *address)
{
	struct sockaddr_in a;
	int s;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket error\n");
		close(s);
		return -1;
	}

	memset(&a, 0, sizeof(a));
	a.sin_port = htons(connect_port);
	a.sin_family = AF_INET;

	if (!inet_aton(address, (struct in_addr *) &a.sin_addr.s_addr)) {
		fprintf(stderr, "bad IP address format\n");
		close(s);
		return -1;
	}

	if (connect(s, (struct sockaddr *) &a, sizeof(a)) == -1) {
		fprintf(stderr, "connect()\n");
		shutdown(s, SHUT_RDWR);
		close(s);
		return -1;
	}
	return s;
}


static int
handle_request(char *buf, int nread, int clifd, struct sockaddr_in *client_address)
{

    if(buf[0] == 4){
        return 1;
    }
    if (send(clifd, buf, nread, 0) < 0){
        fprintf(stderr, "send_fd error");
        return 1;
    }
        
    printf("sent fd %d %s\n", clifd, inet_ntoa(client_address->sin_addr));
    return 0;
}

static void loop(int port)
{
    int     i, n, maxfd, maxi, listenfd, clifd, nread;
    char    buf[MAXLINE];
    uid_t   uid;
    fd_set  rset, allset;
	struct sockaddr_in client_address;
	unsigned int l;

    FD_ZERO(&allset);

    /* obtain fd to listen for client requests on */
    if ((listenfd = listen_socket(port)) < 0)
        fprintf(stderr, "serv_listen error\n");
    FD_SET(listenfd, &allset);
    maxfd = listenfd;
    maxi = -1;

    for ( ; ; ) {
        rset = allset;  /* rset gets modified each time around */
        if ((n = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0)
            fprintf(stderr, "select error\n");

        if (FD_ISSET(listenfd, &rset)) {
            /* accept new client request */
            if ((clifd = accept(listenfd, (struct sockaddr *)&client_address, &l)) < 0)
                fprintf(stderr, "serv_accept error: %d\n", clifd);
            i = client_add(clifd, &client_address);
            FD_SET(clifd, &allset);
            if (clifd > maxfd)
                maxfd = clifd;  /* max fd for select() */
            if (i > maxi)
                maxi = i;   /* max index in client[] array */


            continue;
        }

        for (i = 0; i <= maxi; i++) {   /* go through client[] array */
            if ((clifd = client[i].fd) < 0)
                continue;
            if (FD_ISSET(clifd, &rset)) {
                /* read argument buffer from client */
                if ((nread = read(clifd, buf, MAXLINE)) < 0) {
                    fprintf(stderr, "read error on fd %d\n", clifd);
                    client_del(clifd);  /* client has closed cxn */
                    FD_CLR(clifd, &allset);
                    close(clifd);                    
                } else if (nread == 0) {
                    client_del(clifd);  /* client has closed cxn */
                    FD_CLR(clifd, &allset);
                    close(clifd);
                } else {    /* process client's request */
                    if(handle_request(buf, nread, clifd, &client[i].addr)){
                        client_del(clifd);  /* client has closed cxn */
                        FD_CLR(clifd, &allset);
                        close(clifd);
                    }

                }
            }
        }
	}
}

	int
main(int argc, char *argv[])
{

	if (argc != 2) {
		fprintf(stderr, "Usage\n\t%s <listen-port> \n",
				argv[0]);
		exit(EXIT_FAILURE);
	}


	loop(atoi(argv[1]));

	exit(EXIT_SUCCESS);
}
