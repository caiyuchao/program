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
#include <stdarg.h>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include "sample_nl.h"

#ifdef HAVE_LIBTINY
#define nl_handle nl_sock
#define nl_handle_alloc nl_socket_alloc
#define nl_handle_destroy nl_socket_free
#endif

#include "genl.c" 
enum {
	L_CRIT,
	L_ERR,
	L_WARNING,
	L_NOTICE,
	L_INFO,
	L_DEBUG
};


static void __log_message(int priority, const char *func, int line, const char *format, ...);

#define dlog(format, ...) do { \
		if (L_DEBUG > log_level) \
			break; \
		__log_message(L_DEBUG, __func__, __LINE__, format, ## __VA_ARGS__); \
	} while (0)
#define nlog(format, ...) do { \
		if (L_NOTICE > log_level) \
			break; \
		__log_message(L_NOTICE, __func__, __LINE__, format, ## __VA_ARGS__); \
	} while (0)
#define elog(format, ...) do { \
		if (L_ERR > log_level) \
			break; \
		__log_message(L_ERR, __func__, __LINE__, format, ## __VA_ARGS__); \
	} while (0)

int			nl_sd;
uint32_t	pid;
uint32_t	nl_grp_id = 0;
uint16_t	nl_family_id = 0;
const char	*nl_grp_name = NULL;
int			log_level = L_DEBUG;

/* Policy used for attributes in nested attribute SAMPLE_ECHO_ATTR */
struct nla_policy sample_echo_policy[SAMPLE_ECHO_ATTR_MAX + 1] = {
	[SAMPLE_ECHO_ATTR_INFO] = {.type = NLA_NESTED},
	[SAMPLE_ECHO_ATTR_DATA] = {.type = NLA_STRING, .maxlen = 1024},
};

struct nla_policy sample_info_policy[SAMPLE_INFO_ATTR_MAX + 1] = {
	[SAMPLE_INFO_ATTR_X] = {.type = NLA_U32},
	[SAMPLE_INFO_ATTR_Y] = {.type = NLA_U32},
};


static void __log_message(int priority, const char *func, int line, const char *format, ...)
{
	char buf[512];
	va_list vl;
	snprintf(buf, sizeof(buf), "%s(%d): %s", func, line, format);

	va_start(vl, format);
	vfprintf(stderr, buf, vl);
	va_end(vl);
}

static void sample_nl_dump(struct sample_nla *sn){
	printf("x:%d y:%d data:%s\n", sn->info.x, sn->info.y, sn->data);
}

static int sample_nl_cb(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlattr *attrs_echo[SAMPLE_ECHO_ATTR_MAX + 1];
	struct nlattr *attrs_info[SAMPLE_INFO_ATTR_MAX + 1];
	struct sample_nla sn;


	if (genlmsg_parse(nlh, 0, attrs_echo, SAMPLE_ECHO_ATTR_MAX,
				sample_echo_policy) != 0){
		return -1;
	}

	if (!attrs_echo[SAMPLE_ECHO_ATTR_INFO] || !attrs_echo[SAMPLE_ECHO_ATTR_DATA]){
		elog("attrs null\n");
		return -1;
	}

	strncpy(sn.data, nla_get_string(attrs_echo[SAMPLE_ECHO_ATTR_DATA]), sizeof(sn.data));

	if(nla_parse_nested(attrs_info, SAMPLE_INFO_ATTR_MAX, 
				attrs_echo[SAMPLE_ECHO_ATTR_INFO], sample_info_policy))
		return -1;

	sn.info.x = nla_get_u32(attrs_info[SAMPLE_INFO_ATTR_X]);
	sn.info.y = nla_get_u32(attrs_info[SAMPLE_INFO_ATTR_Y]);
	sample_nl_dump(&sn);
	return NL_OK;
}


static void show_usage(void)
{
    printf("usage: prog_name <options>...\n"
           "-h : show usage\n"
    );
}

static struct nl_handle *init_netlink(void)
{
	struct nl_handle *handle;
	int ret, multicast_id;

	handle = nl_handle_alloc();
	if (!handle){
		elog("Cannot allocate netlink handle!\n");
		return NULL;
	}
	nl_disable_sequence_check(handle);
	ret = genl_connect(handle);
	if (ret < 0){
		elog("Cannot connect to netlink socket\n");
		return NULL;
	}

	/*
	familyid = genl_ctrl_resolve(handle, SAMPLE_NL_FAMILY_NAME);
	if (!familyid){
		elog("Cannot resolve family name(%s)\n", SAMPLE_NL_FAMILY_NAME);
		return NULL;
	}
	dlog("familyid %d\n", familyid);
	*/

	multicast_id = nl_get_multicast_id(handle, SAMPLE_NL_FAMILY_NAME, SAMPLE_NL_GRP_NAME);
	if (multicast_id < 0){
		elog("Cannot resolve grp name(%d)\n", SAMPLE_NL_GRP_NAME);
		return NULL;
	}

	ret = nl_socket_add_membership(handle, multicast_id);
	if (ret < 0){
		elog("Cannot join fs multicast group\n");
		return NULL;
	}

	ret = nl_socket_modify_cb(handle, NL_CB_VALID, NL_CB_CUSTOM,
			sample_nl_cb, NULL);
	if (ret < 0){
		elog("Cannot register callback for"
			 " netlink messages\n");
		return NULL;
	}

	return handle;
}

static void run(struct nl_handle *nhandle)
{
	int ret;

	while (1) {
		dlog("nl_recvmsgs_default .. \n");
		ret = nl_recvmsgs_default(nhandle);
		if (ret < 0)
			dlog("Failed to read or parse netlink"
				" message: %s\n", strerror(-ret));
	}
}

int main(int argc, char *argv[])
{
	int arg = -1;
	struct nl_handle *nhandle;

	while (-1 != (arg = getopt(argc, argv, "h")))
	{
		switch (arg)
		{
			case 'h':
				show_usage();
				return 0;
			default:
				printf("option %c is invalid, ignored!\n", optopt);
				return 0;
		}
	}

	nhandle = init_netlink();
	if(!nhandle)
		return 1;
	dlog("running ... \n");

	run(nhandle);
	nl_handle_destroy(nhandle);

	return 0;
}
