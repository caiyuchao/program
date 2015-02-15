#ifndef __SAMPLE_NL_H__
#define __SAMPLE_NL_H__

#define SAMPLE_NL_FAMILY_NAME "sample_netlink"
#define SAMPLE_NL_GRP_NAME "notify"
#define SAMPLE_NL_VERSION   0x1

/* network order */
struct sample_info {
	uint32_t x;
	uint32_t y;
};

struct sample_nla {
	struct sample_info info;
	char data[2048];
};

/* support command */
enum {
	SAMPLE_NL_CMD_UNSPEC = 0,
	SAMPLE_NL_CMD_ECHO,
	SAMPLE_NL_CMD_NOTIFY,
	__SAMPLE_NL_CMD_MAX,
};
#define SAMPLE_NL_CMD_MAX (__SAMPLE_NL_CMD_MAX - 1)

/* support cmd attrs */
enum {
	SAMPLE_CMD_ATTR_UNSPEC = 0,
	SAMPLE_CMD_ATTR_ECHO,
	__SAMPLE_CMD_ATTR_MAX,
};
#define SAMPLE_CMD_ATTR_MAX (__SAMPLE_CMD_ATTR_MAX - 1)

/* support echo attrs */
enum {
	SAMPLE_ECHO_ATTR_UNSPEC = 0,
	SAMPLE_ECHO_ATTR_INFO,
	SAMPLE_ECHO_ATTR_DATA,
	__SAMPLE_ECHO_ATTR_MAX,
};
#define SAMPLE_ECHO_ATTR_MAX (__SAMPLE_ECHO_ATTR_MAX - 1)

/* support info attrs */
enum {
	SAMPLE_INFO_ATTR_UNSPEC = 0,
	SAMPLE_INFO_ATTR_X,
	SAMPLE_INFO_ATTR_Y,
	__SAMPLE_INFO_ATTR_MAX,
};
#define SAMPLE_INFO_ATTR_MAX (__SAMPLE_INFO_ATTR_MAX - 1)

#endif /* __SAMPLE_NL_H__ */
