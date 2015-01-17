#ifndef __MI_NETLINK_H__
#define __MI_NETLINK_H__

#define MI_NETLINK_NL_FAMILY_NAME "minetlink"
#define MI_NETLINK_NL_GRP_NAME "mapping"

/* network order */
struct mi_http_info {
	uint32_t sip;
	uint32_t dip;
	uint16_t sport;
	uint16_t dport;
};

enum {
	MI_NETLINK_CMD_UNSPEC = 0,
	MI_NETLINK_CMD_SKB_DUMP,
	MI_NETLINK_CMD_MAX
};

enum {
	MI_NETLINK_ATTR_UNSPEC = 0,
	__MI_NETLINK_ATTR_MAX,
};
#define MI_NETLINK_ATTR_MAX (__MI_NETLINK_ATTR_MAX - 1)

enum {
	MI_NETLINK_TYPE_UNSPEC = 0,
	MI_NETLINK_TYPE_HTTP_INFO,
	MI_NETLINK_TYPE_HTTP_DATA,
	__MI_NETLINK_TYPE_MAX
};
#define MI_NETLINK_TYPE_MAX (__MI_NETLINK_TYPE_MAX - 1)

#endif /* __MI_NETLINK_H__ */
