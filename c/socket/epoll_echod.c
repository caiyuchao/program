/* yubo@yubo.org
 * 2014/10/19 11:04
*/
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
#include <fcntl.h>
#include <sys/epoll.h>


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
Client *
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
            return &client[i];  /* return index in client[] array */
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
client_del(void *ptr)
{
    Client *cli = (Client *) ptr;
    fprintf(stdout, "closed: client_address %s, fd %d\n",
      inet_ntoa(cli->addr.sin_addr), cli->fd);    
    cli->fd = -1;
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

#define MAX_EVENTS 10

void setnonblocking(int sock)
{
     int opts;
     opts=fcntl(sock,F_GETFL);
     if(opts<0)
     {
          perror("fcntl(sock,GETFL)");
          exit(1);
     }
     opts = opts|O_NONBLOCK;
     if(fcntl(sock,F_SETFL,opts)<0)
     {
          perror("fcntl(sock,SETFL,opts)");
          exit(1);
     }  
}

/*
typedef union epoll_data
{
  void *ptr;
  int fd; 
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event
{
  uint32_t events;  
  epoll_data_t data;   
};
*/

static void loop(int port)
{
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd, n, nread;
    struct sockaddr_in client_address;
    unsigned int addrlen;
    char    buf[MAXLINE];


    epollfd = epoll_create(MAX_EVENTS);
    if (epollfd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    if ((listen_sock = listen_socket(port)) < 0)
        fprintf(stderr, "serv_listen error\n");


    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
       perror("epoll_ctl: listen_sock");
       exit(EXIT_FAILURE);
    }

   for (;;) {
       nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
       if (nfds == -1) {
           perror("epoll_pwait");
           exit(EXIT_FAILURE);
       }

       for (n = 0; n < nfds; ++n) {
           if (events[n].data.fd == listen_sock) {
               conn_sock = accept(listen_sock,
                               (struct sockaddr *) &client_address, &addrlen);
               if (conn_sock == -1) {
                   perror("accept");
                   exit(EXIT_FAILURE);
               }


               ev.data.ptr = client_add(conn_sock, &client_address);
               setnonblocking(conn_sock);
               ev.events = EPOLLIN | EPOLLET;
               if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                           &ev) == -1) {
                   perror("epoll_ctl: conn_sock");
                   exit(EXIT_FAILURE);
               }
           } else {
               /* do_use_fd(events[n].data.fd); */

               Client *cli = (Client *)events[n].data.ptr;

                /* read argument buffer from client */
                if ((nread = read(cli->fd, buf, MAXLINE)) < 0) {
                    fprintf(stderr, "read error on fd %d\n", cli->fd);
                    close(cli->fd); 
                    client_del(cli);  /* client has closed cxn */
                } else if (nread == 0) {
                    close(cli->fd); 
                    client_del(cli);  /* client has closed cxn */
                } else {    /* process client's request */
                    if(handle_request(buf, nread, cli->fd, &cli->addr)){
                        close(cli->fd); 
                        client_del(cli);  /* client has closed cxn */
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
