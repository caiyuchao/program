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

static void
client_alloc(void)      /* alloc more entries in the client[] array */
{
    int     i;

    if (client == NULL)
        client = malloc(NALLOC * sizeof(Client));
    else
        client = realloc(client, (client_size+NALLOC)*sizeof(Client));
    if (client == NULL)
        err_sys("can't alloc for client array");

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
            client[i].fd = -1;
            return;
        }
    }
    perror("can't find client entry for fd %d", fd);
}


	static int
listen_socket(int listen_port)
{
	struct sockaddr_in a;
	int s;
	int yes;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}
	yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
				(char *) &yes, sizeof(yes)) == -1) {
		perror("setsockopt");
		close(s);
		return -1;
	}
	memset(&a, 0, sizeof(a));
	a.sin_port = htons(listen_port);
	a.sin_family = AF_INET;
	if (bind(s, (struct sockaddr *) &a, sizeof(a)) == -1) {
		perror("bind");
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
		perror("socket");
		close(s);
		return -1;
	}

	memset(&a, 0, sizeof(a));
	a.sin_port = htons(connect_port);
	a.sin_family = AF_INET;

	if (!inet_aton(address, (struct in_addr *) &a.sin_addr.s_addr)) {
		perror("bad IP address format");
		close(s);
		return -1;
	}

	if (connect(s, (struct sockaddr *) &a, sizeof(a)) == -1) {
		perror("connect()");
		shutdown(s, SHUT_RDWR);
		close(s);
		return -1;
	}
	return s;
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
        perror("serv_listen error");
    FD_SET(listenfd, &allset);
    maxfd = listenfd;
    maxi = -1;

    for ( ; ; ) {
        rset = allset;  /* rset gets modified each time around */
        if ((n = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0)
            perror("select error");

        if (FD_ISSET(listenfd, &rset)) {
            /* accept new client request */
            if ((clifd = accept(listenfd, (struct sockaddr *)&client_address, &l)) < 0)
                perror("serv_accept error: %d", clifd);
            i = client_add(clifd, uid);
            FD_SET(clifd, &allset);
            if (clifd > maxfd)
                maxfd = clifd;  /* max fd for select() */
            if (i > maxi)
                maxi = i;   /* max index in client[] array */
            log_msg("new connection: uid %d, fd %d", uid, clifd);
            continue;
        }

        for (i = 0; i <= maxi; i++) {   /* go through client[] array */
            if ((clifd = client[i].fd) < 0)
                continue;
            if (FD_ISSET(clifd, &rset)) {
                /* read argument buffer from client */
                if ((nread = read(clifd, buf, MAXLINE)) < 0) {
                    log_sys("read error on fd %d", clifd);
                } else if (nread == 0) {
                    log_msg("closed: uid %d, fd %d",
                      client[i].uid, clifd);
                    client_del(clifd);  /* client has closed cxn */
                    FD_CLR(clifd, &allset);
                    close(clifd);
                } else {    /* process client's request */
                    handle_request(buf, nread, clifd, client[i].uid);
                }
            }
        }
	}
}

#define SHUT_FD1 do {               \
	if (fd1 >= 0) {                 \
		shutdown(fd1, SHUT_RDWR);   \
		close(fd1);                 \
		fd1 = -1;                   \
	}                               \
} while (0)

#define SHUT_FD2 do {               \
	if (fd2 >= 0) {                 \
		shutdown(fd2, SHUT_RDWR);   \
		close(fd2);                 \
		fd2 = -1;                   \
	}                               \
} while (0)

#define BUF_SIZE 1024

	int
main(int argc, char *argv[])
{
	int h;
	int fd1 = -1, fd2 = -1;
	char buf1[BUF_SIZE], buf2[BUF_SIZE];
	int buf1_avail, buf1_written;
	int buf2_avail, buf2_written;

	if (argc != 2) {
		fprintf(stderr, "Usage\n\t%s <listen-port> ",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	signal(SIGPIPE, SIG_IGN);

	h = listen_socket(atoi(argv[1]));
	if (h == -1)
		exit(EXIT_FAILURE);

	for (;;) {
		int r, nfds = 0;
		fd_set rd, wr, er;

		FD_ZERO(&rd);
		FD_ZERO(&wr);
		FD_ZERO(&er);
		FD_SET(h, &rd);
		nfds = max(nfds, h);
		if (fd1 > 0 && buf1_avail < BUF_SIZE) {
			FD_SET(fd1, &rd);
			nfds = max(nfds, fd1);
		}
		if (fd2 > 0 && buf2_avail < BUF_SIZE) {
			FD_SET(fd2, &rd);
			nfds = max(nfds, fd2);
		}
		if (fd1 > 0 && buf2_avail - buf2_written > 0) {
			FD_SET(fd1, &wr);
			nfds = max(nfds, fd1);
		}
		if (fd2 > 0 && buf1_avail - buf1_written > 0) {
			FD_SET(fd2, &wr);
			nfds = max(nfds, fd2);
		}
		if (fd1 > 0) {
			FD_SET(fd1, &er);
			nfds = max(nfds, fd1);
		}
		if (fd2 > 0) {
			FD_SET(fd2, &er);
			nfds = max(nfds, fd2);
		}

		r = select(nfds + 1, &rd, &wr, &er, NULL);

		if (r == -1 && errno == EINTR)
			continue;

		if (r == -1) {
			perror("select()");
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(h, &rd)) {
			unsigned int l;
			struct sockaddr_in client_address;

			memset(&client_address, 0, sizeof(client_address));
			r = accept(h, (struct sockaddr *) &client_address, &l);
			if (r == -1) {
				perror("accept() error");
			} else {
				SHUT_FD1;
				SHUT_FD2;
				buf1_avail = buf1_written = 0;
				buf2_avail = buf2_written = 0;
				fd1 = r;
				fd2 = connect_socket(forward_port, argv[3]);
				if (fd2 == -1)
					SHUT_FD1;
				else
					printf("connect from %s\n",
							inet_ntoa(client_address.sin_addr));
			}
		}

		/* NB: read oob data before normal reads */

		if (fd1 > 0)
			if (FD_ISSET(fd1, &er)) {
				char c;

				r = recv(fd1, &c, 1, MSG_OOB);
				if (r < 1)
					SHUT_FD1;
				else
					send(fd2, &c, 1, MSG_OOB);
			}
		if (fd2 > 0)
			if (FD_ISSET(fd2, &er)) {
				char c;

				r = recv(fd2, &c, 1, MSG_OOB);
				if (r < 1)
					SHUT_FD2;
				else
					send(fd1, &c, 1, MSG_OOB);
			}
		if (fd1 > 0)
			if (FD_ISSET(fd1, &rd)) {
				r = read(fd1, buf1 + buf1_avail,
						BUF_SIZE - buf1_avail);
				if (r < 1)
					SHUT_FD1;
				else
					buf1_avail += r;
			}
		if (fd2 > 0)
			if (FD_ISSET(fd2, &rd)) {
				r = read(fd2, buf2 + buf2_avail,
						BUF_SIZE - buf2_avail);
				if (r < 1)
					SHUT_FD2;
				else
					buf2_avail += r;
			}
		if (fd1 > 0)
			if (FD_ISSET(fd1, &wr)) {
				r = write(fd1, buf2 + buf2_written,
						buf2_avail - buf2_written);
				if (r < 1)
					SHUT_FD1;
				else
					buf2_written += r;
			}
		if (fd2 > 0)
			if (FD_ISSET(fd2, &wr)) {
				r = write(fd2, buf1 + buf1_written,
						buf1_avail - buf1_written);
				if (r < 1)
					SHUT_FD2;
				else
					buf1_written += r;
			}

		/* check if write data has caught read data */

		if (buf1_written == buf1_avail)
			buf1_written = buf1_avail = 0;
		if (buf2_written == buf2_avail)
			buf2_written = buf2_avail = 0;

		/* one side has closed the connection, keep
		   writing to the other side until empty */

		if (fd1 < 0 && buf1_avail - buf1_written == 0)
			SHUT_FD2;
		if (fd2 < 0 && buf2_avail - buf2_written == 0)
			SHUT_FD1;
	}
	exit(EXIT_SUCCESS);
}
