/*
 * Yu Bo<yubo@yubo.org> 
 * 2015-02-03
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/genetlink.h>

#include "sample_nl.h"

#define GENLMSG_DATA(glh)	((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh)	(NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na)		((void *)((char*)(na) + NLA_HDRLEN))
#define NLA_PAYLOAD_LEN(len)	(len - NLA_HDRLEN)

/* Maximum size of response requested or message sent */
#define MAX_MSG_SIZE	2048

/* MAX url length */
#define MAX_URL_LEN	 512

/* Please turn off debug to test performance test */
#define __DEBUG_MSG__

int		nl_sd;
uint32_t	pid;
uint32_t	nl_grp_id = 0;
uint16_t	nl_family_id = 0;
const char * nl_grp_name = NULL;

struct msgtemplate {
	struct nlmsghdr n __attribute__ ((aligned(NLMSG_ALIGNTO)));
	union {
		struct {
			struct genlmsghdr g __attribute__
						((aligned(NLMSG_ALIGNTO)));
			char buf[MAX_MSG_SIZE];
		};
		struct nlmsgerr nlerr;
	};
};

#define LOG printf

#ifdef __DEBUG_MSG__
static inline void print_timestamp(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("%s, %usecs, %umicrosec\n", __FUNCTION__, (uint32_t)tv.tv_sec, (uint32_t)tv.tv_usec);
}
#else
static void print_timestamp(void) {}
#endif


/* webfilter string type : host or uri */
enum
{
	WEBFILTER_STR_URI = 0,
	WEBFILTER_STR_HOST,
	WEBFILTER_STR_REFER,
};


static void
parse_groups(struct nlattr *na, int tot_len)
{
	int	len;
	int	grp_len;
	int	aggr_len;
	struct nlattr *grp_na;
	char group_name[GENL_NAMSIZ] = {0};
	int group_id = -1;

	len = 0;
	while (len < tot_len) {
		len += NLA_ALIGN(na->nla_len);
		printf("grp #%02d\n", na->nla_type);
#if 0
		if (na->nla_type > 1) {
			/* only one group supported for now */
			//na = (struct nlattr *) ((char *) na + len);
			//continue;
			printf("!!multi group, loop to find!!\n");
		}
#endif
		aggr_len = NLA_PAYLOAD_LEN(na->nla_len);
		grp_na = (struct nlattr *) NLA_DATA(na);
		grp_len = 0;
		while (grp_len < aggr_len) {
			grp_len += NLA_ALIGN(grp_na->nla_len);
			switch (grp_na->nla_type) {
				case CTRL_ATTR_MCAST_GRP_ID:
					group_id = *(uint32_t *) NLA_DATA(grp_na);
					printf("grp id = %d\n", nl_grp_id);
					break;
				case CTRL_ATTR_MCAST_GRP_NAME:
					strcpy(group_name, (char *)NLA_DATA(grp_na));
					printf("grp name %s\n", group_name);
					break;
				default:
					printf("Unknown grp nested attr %d\n", 	grp_na->nla_type);
					break;
			}
			grp_na = (struct nlattr *) ((char *) grp_na + grp_len);
		}
		if (-1 != group_id && strlen(group_name) > 0) {
			if (0 == strcmp(group_name, nl_grp_name)) {
				printf("find grp name:%s, id=%d\n", group_name, group_id);
				nl_grp_id = group_id;
				break;
			}
		}
		na = (struct nlattr *) ((char *) na + len);
	}
}

/*
 *           Netlink interface
 */
static int send_cmd(int sd, __u16 nlmsg_type, __u8 genl_cmd, __u16 nla_type,
		void *nla_data, int nla_len, uint16_t flags)
{
	struct nlattr *na;
	struct sockaddr_nl nladdr;
	int r, buflen;
	char *buf;

	struct msgtemplate msg;

	msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	msg.n.nlmsg_type = nlmsg_type;
	msg.n.nlmsg_flags = flags;
	msg.n.nlmsg_seq = 0;
	msg.n.nlmsg_pid = pid;
	msg.g.cmd = genl_cmd;
	msg.g.version = 0x1;
	na = (struct nlattr *) GENLMSG_DATA(&msg);
	na->nla_type = nla_type;
	na->nla_len = nla_len + NLA_HDRLEN;
	memcpy(NLA_DATA(na), nla_data, nla_len);
	msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

	buf = (char *) &msg;
	buflen = msg.n.nlmsg_len ;
	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	while ((r = sendto(sd, buf, buflen, 0, (struct sockaddr *) &nladdr,
					sizeof(nladdr))) < buflen) {
		printf("sending (r=%d)\n", r);
		if (r > 0) {
			buf += r;
			buflen -= r;
		} else if (errno != EAGAIN)
			return r;
	}
	printf("msg sent (r=%d)\n", r);

	return 0;
}
static int get_family_id(int sd)
{
	struct msgtemplate msg;
	int	len;
	int	recv_len;
	int	rc;
	struct nlattr *na;

	rc = send_cmd(sd, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
			CTRL_ATTR_FAMILY_NAME, SAMPLE_NL_FAMILY_NAME,
			strlen(SAMPLE_NL_FAMILY_NAME)+1, NLM_F_REQUEST);
	if (rc < 0) {
		printf("Error sending family cmd (%d:%s)\n",
				errno, strerror(errno));
		return -1;
	}
	recv_len = recv(sd, &msg, sizeof(msg), 0);
	if (msg.n.nlmsg_type == NLMSG_ERROR) {
		printf("Error: recv family error msg\n");
		return -1;
	}
	if (recv_len < 0) {
		printf("Error: recv family (%d)\n", recv_len);
		return -1;
	}
	if (!NLMSG_OK((&msg.n), recv_len)) {
		printf("Error: recv family msg nok\n");
		return -1;
	}

	len = 0;
	recv_len = GENLMSG_PAYLOAD(&msg.n);
	na = (struct nlattr *) GENLMSG_DATA(&msg);
	while (len < recv_len) {
		len += NLA_ALIGN(na->nla_len);
		switch (na->nla_type) {
			case CTRL_ATTR_FAMILY_ID:
				nl_family_id = *(uint16_t *) NLA_DATA(na);
				printf("family id:%d\n", nl_family_id);
				break;
			case CTRL_ATTR_MCAST_GROUPS:
				parse_groups(NLA_DATA(na),
						NLA_PAYLOAD_LEN(na->nla_len));
				break;
			case CTRL_ATTR_FAMILY_NAME:
			case CTRL_ATTR_VERSION:
			case CTRL_ATTR_HDRSIZE:
			case CTRL_ATTR_MAXATTR:
			case CTRL_ATTR_OPS:
				printf("Unused family attr %d\n", na->nla_type);
				break;
			default:
				printf("Unknown family attr %d\n", na->nla_type);
				break;
		}
		na = (struct nlattr *) (GENLMSG_DATA(&msg) + len);
	}

	return nl_family_id;
}

static int open_nl_socket(void)
{
	int fd;
	struct sockaddr_nl local;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
	if (fd < 0) {
		printf("Error opening socket (%d)\n", errno);
		return -1;
	}
	printf("fd:%d\n", fd);

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;

	if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
		printf("Error binding socket (%d)\n", errno);
		close(fd);
		return -1;
	}
	printf("bind done\n");

	/* Retrieve netlink family Id */
	nl_family_id = get_family_id(fd);
	if (nl_family_id < 0) {
		printf("Error: Could not retreive netlink family id\n");
		close(fd);
		return -1;
	}

	return fd;
}

static int wait_packets(int sd)
{
	int flags;
	int ret;
	int recv_len;
	int len;
	int err;
	fd_set rfds;
	struct nlattr *na;
	struct msgtemplate msg;
	long xxxx = 0;
	long errcnt = 0;
	uint32_t info_x, info_y;
	char data[1024];
	
	flags = fcntl(sd, F_GETFL);
	fcntl(sd, F_SETFL, flags | O_NONBLOCK);

	err = 0;
	do {
		FD_ZERO(&rfds);
		FD_SET(sd, &rfds);

		/* Monitoring, no timeout */
		ret = select(sd+1, &rfds, NULL, NULL, NULL);

		if (ret < 0) {
			if (errno == EINTR)
				continue;
			perror("select()");
			err = ret;
			break;
		} else if (ret == 0) {
			err = -ETIMEDOUT;
			break;
		}
		if (!FD_ISSET(sd, &rfds))
			continue;

		recv_len = recv(nl_sd, &msg, sizeof(msg), 0);
		if (recv_len < 0) {
			printf("nonfatal reply error: errno %d\n", recv_len);
			err = errno;
			errcnt++;
			printf("fail cnt %ld\n", errcnt);
		}
		recv_len = GENLMSG_PAYLOAD(&msg.n);
		na = (struct nlattr *) GENLMSG_DATA(&msg);

		printf("%s, recv genl msg, cmd=%d\n", __FUNCTION__, msg.g.cmd);
		print_timestamp();
		len = 0;
		while (len < recv_len) {
			len += NLA_ALIGN(na->nla_len);
			switch (na->nla_type) {
				case SAMPLE_ECHO_ATTR_INFO:
					{
						struct nlattr * na_info;
						int len_info = 0;
						na_info = (struct nlattr *)NLA_DATA(na);
						while(len_info < len) {
							len_info += NLA_ALIGN(na_info->nla_len);
							switch(na_info->nla_type){
								case SAMPLE_INFO_ATTR_X:
									info_x = *(uint32_t *)NLA_DATA(na_info);
									printf("notify x:%u\n", info_x);
									break;
								case SAMPLE_INFO_ATTR_Y:
									info_y = *(uint32_t *)NLA_DATA(na_info);
									printf("notify y:%u\n", info_y);
									break;
								default:
									printf("Unknown na_info->nla_type %d\n", na_info->nla_type);
									break;
							}
							na_info = (struct nlattr *) (GENLMSG_DATA(&msg) + len_info);
						}
					}
					break;
				case SAMPLE_ECHO_ATTR_DATA:
					strcpy(data, (char *)NLA_DATA(na));
					printf("notify data:[%s]\n", data);
					break;
				default:
					printf("Unknown na->nla_type %d\n", na->nla_type);
					break;
			}
			na = (struct nlattr *) (GENLMSG_DATA(&msg) + len);
		}
		xxxx++;
		if (xxxx % 100 == 0)
			printf("Get message %ld\n", xxxx);
	} while (1);

	/* restore previous attributes */
	fcntl(sd, F_SETFL, flags);
	return err;
}

static void show_usage(void)
{
    printf("usage: prog_name <options>...\n"
           "-h : show usage\n"
           "-g : specify group name, current mapping or security\n\n"
    );
}

int main(int argc, char *argv[])
{
	int arg = -1;
	int rcvbuf_size = 1024*100;

	if (argc < 2) {
		show_usage();
		return 0;
	}
	while (-1 != (arg = getopt(argc, argv, "hg:")))
	{
		switch (arg)
		{
			case 'h':
				show_usage();
				return 0;
			case 'g':
				nl_grp_name = optarg;
				printf("specify group name=%s\n\n", nl_grp_name);
				break;
			case ':':
				printf("option %c requires an argument!\n", optopt);
				return 0;
			case '?':
			default:
				printf("option %c is invalid, ignored!\n", optopt);
				return 0;
		}
	}

	if (NULL == nl_grp_name) {
		printf("using group name [security] as default!\n");
	}

	pid = getpid();
	nl_sd = open_nl_socket();
	if (nl_sd < 0) {
		printf("%s: failed to create nl socket\n", __func__);
		exit(EXIT_FAILURE);
	}

#ifndef SOL_NETLINK
	/* normally defined in bits/socket.h but not available in some
	 * toolchains.
	 */
#define SOL_NETLINK 270
#endif

	/* Monitor netlink socket, we should never return from this */
	setsockopt(nl_sd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
			&nl_grp_id, sizeof(nl_grp_id));
	setsockopt(nl_sd, SOL_SOCKET, SO_RCVBUF, (const void *)&rcvbuf_size, sizeof(int));

	wait_packets(nl_sd);

	close(nl_sd);
	return 0;
}
