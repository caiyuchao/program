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

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include "sample_nl.h"

#ifdef HAVE_LIBTINY
#define nl_handle nl_sock
#define nl_handle_alloc nl_socket_alloc
#define nl_handle_destroy nl_socket_free
#endif

static struct nl_handle *sock = NULL;
static int family;


char buff[1024];

/* Policy used for attributes in nested attribute SAMPLE_ECHO_ATTR */
struct nla_policy sample_echo_policy[SAMPLE_ECHO_ATTR_MAX + 1] = {
	[SAMPLE_ECHO_ATTR_INFO] = {.type = NLA_NESTED},
	[SAMPLE_ECHO_ATTR_DATA] = {.type = NLA_STRING, .maxlen = 1024},
};

struct nla_policy sample_info_policy[SAMPLE_INFO_ATTR_MAX + 1] = {
	[SAMPLE_INFO_ATTR_X] = {.type = NLA_U32},
	[SAMPLE_INFO_ATTR_Y] = {.type = NLA_U32},
};

static struct nl_msg *sample_nl_message(int cmd, int flags)
{
	struct nl_msg *msg;

	msg = nlmsg_alloc();
	if (!msg)
		return NULL;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, flags,
			cmd, SAMPLE_NL_VERSION);

	return msg;
}

static int sample_nl_noop_cb(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

static int sample_nl_send_message(struct nl_msg *msg, nl_recvmsg_msg_cb_t func, void *arg)
{
	int err = EINVAL;

	sock = nl_handle_alloc();
	if (!sock) {
		nlmsg_free(msg);
		return -1;
	}

	if (genl_connect(sock) < 0)
		goto fail_genl;

	family = genl_ctrl_resolve(sock, SAMPLE_NL_FAMILY_NAME);
	if (family < 0)
		goto fail_genl;

	/* To test connections and set the family */
	if (msg == NULL) {
		nl_handle_destroy(sock);
		sock = NULL;
		return 0;
	}

	if (nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, func, arg) != 0)
		goto fail_genl;

	if (nl_send_auto_complete(sock, msg) < 0)
		goto fail_genl;

	if ((err = -nl_recvmsgs_default(sock)) > 0)
		goto fail_genl;

	nlmsg_free(msg);

	nl_handle_destroy(sock);

	return 0;

fail_genl:
	nl_handle_destroy(sock);
	sock = NULL;
	nlmsg_free(msg);
	errno = err;
	return -1;
}



static int sample_nl_init(void){
	if (sample_nl_send_message(NULL, NULL, NULL)) {
		fprintf(stderr, "genl send message error\n");
		return -1;
	}
	return 0;
}

static int sample_nl_exit(void){
	return 0;
}

static void sample_nla_dump(struct sample_nla *sn){
	printf("x:%d y:%d data:%s\n", sn->info.x, sn->info.y, sn->data);
}

static int sample_echo_cb(struct nl_msg *msg, void *arg)
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
		printf("%s %d attrs null\n", __func__, __LINE__);
		return -1;
	}

	strncpy(sn.data, nla_get_string(attrs_echo[SAMPLE_ECHO_ATTR_DATA]), sizeof(sn.data));

	if(nla_parse_nested(attrs_info, SAMPLE_INFO_ATTR_MAX, 
				attrs_echo[SAMPLE_ECHO_ATTR_INFO], sample_info_policy))
		return -1;

	sn.info.x = nla_get_u32(attrs_info[SAMPLE_INFO_ATTR_X]);
	sn.info.y = nla_get_u32(attrs_info[SAMPLE_INFO_ATTR_Y]);
	sample_nla_dump(&sn);
	return NL_OK;
}

static int sample_nl_fill_echo_attr(struct nl_msg *msg, struct sample_nla *sn)
{
	struct nlattr *nl_echo, *nl_info;

	nl_echo = nla_nest_start(msg, SAMPLE_CMD_ATTR_ECHO);
	if (!nl_echo)
		return -1;

	NLA_PUT_STRING(msg, SAMPLE_ECHO_ATTR_DATA, sn->data);

	nl_info = nla_nest_start(msg, SAMPLE_ECHO_ATTR_INFO);
	if (!nl_info)
		return -1;
	NLA_PUT_U32(msg, SAMPLE_INFO_ATTR_X, sn->info.x);
	NLA_PUT_U32(msg, SAMPLE_INFO_ATTR_Y, sn->info.y);
	nla_nest_end(msg, nl_info);

	nla_nest_end(msg, nl_echo);
	return 0;

nla_put_failure:
	return -1;
}


static int  sample_echo(struct sample_nla *sn)
{
	struct nl_msg *msg;

	msg = sample_nl_message(SAMPLE_NL_CMD_ECHO, 0);
	if (!msg)
		return 1;

	if (sample_nl_fill_echo_attr(msg, sn)){
		nlmsg_free(msg);
		return 1;
	}

	if (sample_nl_send_message(msg, sample_echo_cb, NULL)){
		return 1;
	}

	return 0;
}

static int sample_dumpit(void){

	struct nl_msg *msg;

	msg = sample_nl_message(SAMPLE_NL_CMD_ECHO, NLM_F_DUMP);
	if (!msg)
		return 1;

	if (sample_nl_send_message(msg, sample_echo_cb, NULL)){
		return 1;
	}
	return 0;
}

static void show_usage(void)
{
    printf("usage: prog_name <options>...\n"
           "-h : show usage\n"
           "-D : dump it!\n"
           "-x : int x\n"
           "-y : int y\n"
           "-d : string data\n\n"
    );
}

int main(int argc, char *argv[])
{
	int arg = -1;
	int dumpit = 0;
	struct sample_nla sn;

	if (argc < 2) {
		show_usage();
		return 0;
	}

	memset(&sn, 0, sizeof(sn));

	while (-1 != (arg = getopt(argc, argv, "hDx:y:d:")))
	{
		switch (arg)
		{
			case 'h':
				show_usage();
				return 0;
			case 'D':
				dumpit = 1;
				break;
			case 'x':
				sn.info.x = atoi(optarg);
				break;
			case 'y':
				sn.info.y = atoi(optarg);
				break;
			case 'd':
				strcpy(sn.data, optarg);
				break;
			case '?':
			default:
				printf("option %c is invalid, ignored!\n", optopt);
				show_usage();
				return 1;
		}
	}

	if (sample_nl_init()) {
		printf("ip_acc_init failed\n");
		exit(-1);
	}

	if(dumpit){
		sample_dumpit();
	}else if(sn.info.x && sn.info.y && sn.data){
		sample_echo(&sn);
	}else{
		printf("option %c is invalid, ignored!\n", optopt);
		show_usage();
		return 1;
	}

	sample_nl_exit();
	return EXIT_SUCCESS;
}



