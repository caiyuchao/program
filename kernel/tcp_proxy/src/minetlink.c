/**
Miwifi netlink mesage
===========================

Version 0.1
	only support broadcast message.
TODO:
	support all the miwifi related netlink functions
Done by zhangjunwei@xiaomi.com
	2014-03-20
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
#include <linux/genetlink.h>

#include "mi_netlink.h"

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

/*
 *         Security Query
 */

extern int send_pkg_by_udp(char *url, char *refer, char *mac, char *ip);

/*
 *          parse request
 *
 */
#define LOG printf

#ifdef __DEBUG_MSG__
static inline void print_timestamp(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("%s, %dsecs, %dmicrosec\n", __FUNCTION__, tv.tv_sec, tv.tv_usec);
}
#else
static void print_timestamp(void) {}
#endif

/* findend : find end of current request line, end with '\r\n'
 * return value : return end data point
 */
static inline const char *findend(const char *data, const char *tail, int min)
{
	int n = tail - data;
	if (n >= min)
	{
		while (data < tail)
		{
			if (*data == '\r') return data;
			++data;
		}
	}
	return NULL;
}

/* get_strip_uri : trim space of string, extract uri from request
 * char * data   : data begin point
 * char * tail   : data tail point
 * int  * len    : len of uri
 * return value  : uri
 */
static int get_strip_uri(const char **data, const char *tail, int * len)
{
	const char * begin = *data;
	while ((begin < tail) && (*begin == ' ')) ++begin;
	while ((tail > begin) && (*(tail - 1) == ' ')) --tail;

	*len = tail - begin;

	if (*len >= MAX_URL_LEN)
	{
		LOG("===Web filter URI space NOT enough for this package!\n");
		return -1;
	}

	//LOG("len=%d, query uri : ", *len);

	return 0;
}

/* webfilter string type : host or uri */
enum
{
	WEBFILTER_STR_URI = 0,
	WEBFILTER_STR_HOST,
	WEBFILTER_STR_REFER,
};

/* get_strip_string : trim space of string, return actually data point and length
 * int type   : WEBFILTER_STR_URL, WEBFILTER_STR_HOST or WEBFILTER_STR_REFER
 * char ** data : data begin point
 * char * tail  : data tail point
 * int * len    : data actually length
 * return value : data and length
 */
static void get_strip_string(int type, const char **data, const char *tail, int * len)
{
	const char * begin = *data;
	while ((begin < tail) && (*begin == ' ')) ++begin;

	if (WEBFILTER_STR_URI == type || WEBFILTER_STR_REFER == type)
	{
		const char * end = begin;
		while ((tail > end) && (*end != '?'))
		{
			++end;
		}
		tail = end;
	}
	else if (WEBFILTER_STR_HOST == type)
	{
		while ((tail > begin) && (*(tail - 1) == ' '))
		{
			--tail;
		}
	}
	*len = tail - begin;
}

static int parse_http_request(char * data, int len, char * url)
{
	const char * p = data;
	char * tail = p + len;
	const char * uri_data = NULL;
	int uri_len = 0;
	const char * host_data = NULL;
	int host_len = 0;
	int flag_find = 0;

	// POST / HTTP/1.0$$$$
	// GET / HTTP/1.0$$$$   minimum
	// 0123456789012345678
	//      9876543210
	if (((p = findend(data, tail, 18)) == NULL) || (memcmp(p - 9, " HTTP/", 6) != 0))
	{
		return -1;
	}

	// save uri to buf
	uri_data = data + 4;
	//get_strip_string(WEBFILTER_STR_URI, &uri_data, p - 9, &uri_len);
	// support '?' in uri
	if (get_strip_uri(&uri_data, p - 9, &uri_len))
	{
		return -1;
	}
	//printf("urI len : %4d\n", uri_len);

	while (1)
	{
		// find all info, so break
		if (1 == flag_find)
		{
			break;
		}

		data = p + 2;               // skip previous \r\n
		p = findend(data, tail, 8); // p = current line's \r
		if (p == NULL)
		{
			break;
		}

		if (0 == memcmp(data, "Host: ", 6))
		{
			host_data = data + 6;
			get_strip_string(WEBFILTER_STR_HOST, &host_data, p, &host_len);

			if (host_len >= MAX_URL_LEN)
			{
				LOG("%s, host(size:%d) is too long!\n", __FUNCTION__, host_len);
				return -1;
			}

			if (host_len + uri_len >= MAX_URL_LEN)
			{
				LOG("%s, url(size:%d) is too long!\n", __FUNCTION__, host_len + uri_len);
				return -1;
			}

			memcpy(url, host_data, host_len);
			memcpy(url + host_len, uri_data, uri_len);
			url[host_len + uri_len] = '\0';
			printf("urL len : %4d, urL str : %s\n", host_len + uri_len, url);
			flag_find = 1;
		}
	}

	return 0;
}

/*
 *           Netlink interface
 */
int send_cmd(int sd, __u16 nlmsg_type, __u8 genl_cmd, __u16 nla_type,
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

int send_security_response(int sd, __u16 nlmsg_type, __u8 genl_cmd,
		uint16_t flags, void *sec_flag, void *query_id)
{
	struct nlattr *na;
	struct sockaddr_nl nladdr;
	int r, buflen;
	char *buf;
	uint32_t attr_len = 0;

	struct msgtemplate msg;

	msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	msg.n.nlmsg_type = nlmsg_type;
	msg.n.nlmsg_flags = flags;
	msg.n.nlmsg_seq = 0;
	msg.n.nlmsg_pid = pid;
	msg.g.cmd = genl_cmd;
	msg.g.version = 0x1;

	na = (struct nlattr *) GENLMSG_DATA(&msg);
	na->nla_type = MI_NETLINK_ATTR_SEC_QID;
	na->nla_len = sizeof(int) + NLA_HDRLEN;
	memcpy(NLA_DATA(na), query_id, sizeof(int));
	attr_len += na->nla_len;

	na = (struct nlattr *) (GENLMSG_DATA(&msg) + attr_len);
	na->nla_type = MI_NETLINK_ATTR_SECURITY_FLAG;
	na->nla_len = sizeof(int) + NLA_HDRLEN;
	memcpy(NLA_DATA(na), sec_flag, sizeof(int));
	attr_len += na->nla_len;


	msg.n.nlmsg_len += NLMSG_ALIGN(attr_len);

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


	static void
parse_groups(struct nlattr *na, int tot_len)
{
	int	len;
	int	grp_len;
	int	aggr_len;
	struct nlattr *grp_na;
	int find = 0;
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

int get_family_id(int sd)
{
	struct msgtemplate msg;
	int	len;
	int	recv_len;
	int	rc;
	struct nlattr *na;

	rc = send_cmd(sd, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
			CTRL_ATTR_FAMILY_NAME, MI_NETLINK_NL_FAMILY_NAME,
			strlen(MI_NETLINK_NL_FAMILY_NAME)+1, NLM_F_REQUEST);
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

int open_nl_socket(void)
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

int wait_packets(int sd)
{
	int flags;
	int ret;
	int recv_len;
	int len;
	int err;
	fd_set rfds;
	struct nlmsgerr *nl_err;
	struct timeval tv;
	struct nlattr *na;
	struct msgtemplate msg;
	char *http_data;
	struct mi_http_info *http_info;
	long xxxx = 0;
	long errcnt = 0;

	unsigned int query_id = 0;
	char url[MAX_URL_LEN] = {0};
	int sec_flag = 0x0f0f;

#ifdef FIRST_SEND
	printf("%s, first send for test !!!\n", __FUNCTION__);
	err = send_cmd(sd, nl_family_id, MI_NETLINK_CMD_SECURITY_QUERY,
			MI_NETLINK_ATTR_SECURITY_FLAG, (void*)&sec_flag,
			sizeof(int), NLM_F_REQUEST);
	if (err < 0) {
		printf("Error sending result (%d:%s)\n",
				errno, strerror(errno));
		return -1;
	}
#endif

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
			printf("No answer within %lu seconds.\n", tv.tv_sec);
			err = -ETIMEDOUT;
			break;
		}
		if (!FD_ISSET(sd, &rfds))
			continue;

		printf("\n\n========================================\n\n");
		recv_len = recv(nl_sd, &msg, sizeof(msg), 0);
		if (recv_len < 0) {
			printf("nonfatal reply error: errno %d\n", recv_len);
			err = errno;
			errcnt++;
			printf("fail cnt %d\n", errcnt);
			/* TODO check why failed with high pressure. overwrite on rcv buffer? */
			// break;
		}
#ifdef __DEBUG_MSG__
		if (msg.n.nlmsg_type == NLMSG_ERROR ||
				!NLMSG_OK((&msg.n), recv_len)) {
			nl_err = NLMSG_DATA(&msg);
			printf("fatal reply error,  errno %d\n", nl_err->error);
			err = nl_err->error;
			break;
		}
#endif
		recv_len = GENLMSG_PAYLOAD(&msg.n);
		na = (struct nlattr *) GENLMSG_DATA(&msg);

		printf("%s, recv genl msg, cmd=%d\n", __FUNCTION__, msg.g.cmd);
		print_timestamp();
		len = 0;
		while (len < recv_len) {
			len += NLA_ALIGN(na->nla_len);
			switch (na->nla_type) {
				case MI_NETLINK_TYPE_SEC_QID:
					{
						query_id = *(unsigned int *)NLA_DATA(na);
						printf("HTTP query id:%d\n", query_id);
					}
					break;
				case MI_NETLINK_TYPE_HTTP_INFO:
					http_info = (struct http_info *)NLA_DATA(na);
#ifdef __DEBUG_MSG__
					printf("%08x(%u) --> %08x(%u)\n",
							ntohl(http_info->sip), ntohs(http_info->sport),
							ntohl(http_info->dip), ntohs(http_info->dport));
#endif
					break;
				case MI_NETLINK_TYPE_HTTP_DATA:
					http_data = NLA_DATA(na);
#ifdef __DEBUG_MSG__
					printf("Http data len:%d\n", len);
					printf("Http data[%d]:%s\n", len, http_data);
#endif
					if (MI_NETLINK_CMD_SECURITY_QUERY == msg.g.cmd) {
						/* parse REQUEST */
						memset((void*)url, 0, sizeof(url));
						if (parse_http_request(http_data, len, url)) {
							printf("%s, parse http request error, skip this query!!!\n", __FUNCTION__);
							break;
						}

						/* query security center */
						char *mac="78:1D:BA:32:11:06";
						char *ip = "201.201.201.201";
						sec_flag = send_pkg_by_udp(url, url, mac, ip);
						// 2 means evil url
						if (2 == sec_flag) {
							sec_flag = 1;
						} else {
							sec_flag = 0;
						}

						/* send response */
						err = send_security_response(sd, nl_family_id, MI_NETLINK_CMD_SECURITY_QUERY,
								NLM_F_REQUEST, (void*)&sec_flag, (void*)&query_id);
						if (err < 0) {
							printf("Error sending result (%d:%s)\n",
									errno, strerror(errno));
							return -1;
						}
						printf("%s, send query flag=%d, query_id=%d\n", __FUNCTION__, sec_flag, query_id);
						print_timestamp();
					}
					break;
				default:
					printf("Unknown nla_type %d\n", na->nla_type);
					break;
			}
			na = (struct nlattr *) (GENLMSG_DATA(&msg) + len);
		}
		xxxx++;
		if (xxxx % 100 == 0)
			printf("Get message %d\n", xxxx);


	} while (1);

	/* restore previous attributes */
	fcntl(sd, F_SETFL, flags);
	return err;
}

static void show_usage()
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
