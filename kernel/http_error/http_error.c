/*
 * This is a module which is used to replace http .
 *
 * Copyright (C) 2000
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/gfp.h>
#include <linux/ctype.h>

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include <net/dst.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/inet_common.h>
#include <asm/unaligned.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_tcpudp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>

//#define __HTTP_ERROR_DEBUG__
//#define __PERF_DEBUG__
#define __HTTP_ERROR_DISABLE_TS__

extern int web_filter_match(const struct sk_buff *skb);
//extern int init_webfilter_match(void);
//extern void fini_webfilter_match(void);
extern void nf_conntrack_hash_insert(struct nf_conn *ct);
extern void mi_netlink_broadcast(struct sk_buff *orig_skb);
// tcp bypass function
//extern int mi_netlink_init(void);
//extern void mi_netlink_exit(void);

#define HTTP_GET_MIN_LEN	18
/* flow limit, now 512kbytes */
#define FLOW_LIMIT (512*1024)
// network order "GET " and "POST"
static const unsigned int constant_get_string  = __constant_htonl(0x47455420);
static const unsigned int constant_post_string = __constant_htonl(0x504f5354);

/* sysfs API for rule action. */
#define ACTION_SIZE 8
#define ACTION_MASK 0x07
#define ACTION_KIND_2_OFFSET	3

#define GET_ACTION_KIND_1_VAL(v) ((v) & ACTION_MASK)
#define GET_ACTION_KIND_2_VAL(v) (((v) >> ACTION_KIND_2_OFFSET) & ACTION_MASK)

/*
 *      Tcp Proxy Flags
 */
#define HTTP_ERROR_F_IGNORE	0x0001		/* ignore skb */
#define HTTP_ERROR_F_KEEP	0x0002		/* cached skb */

/* proxy server response */
enum {
	SYN_ACK = 1,
	RST_ACK,
};

struct action_1_info {
	union {
		uint32_t ip;	/* network order */
		uint8_t	byte[4];
	};
	uint16_t port;
	uint16_t res;
};

static struct action_1_info action1_vec[ACTION_SIZE] __read_mostly;
static struct ctl_table_header *http_error_action_table_header;

#define __HTTP_ERROR_DEBUG__
#ifdef __HTTP_ERROR_DEBUG__
#define DEBUG_PRINT(fmt, a...) printk(fmt, ##a)
#define DUMP_SKB(skb, in, out) dump_skb(skb, in, out)
#else
#define DEBUG_PRINT(fmt, a...) do { } while(0)
#define DUMP_SKB(skb, in, out) do { } while(0)
#endif

#ifdef __PERF_DEBUG__
#define PERF_DEBUG(fmt, a...) \
    do { \
        struct timeval tv;\
        printk(fmt, ##a); \
        do_gettimeofday(&tv);\
        printk(" timeval(%dsec, %dusec)\n", tv.tv_sec, tv.tv_usec); \
    }while(0)
#else
#define PERF_DEBUG(fmt, a...) do { } while(0)
#endif

#define XT_SYNPROXY_OPT_MSS             0x01
#define XT_SYNPROXY_OPT_WSCALE          0x02
#define XT_SYNPROXY_OPT_SACK_PERM       0x04
#define XT_SYNPROXY_OPT_TIMESTAMP       0x08
#define XT_SYNPROXY_OPT_ECN             0x10

struct synproxy_options {
	u8	options;
	u8	wscale;
	u16	mss;
	u32	tsval;
	u32	tsecr;
};

#define HTTP_SESSION_HASH_SIZE	1024
#define HTTP_SESSION_STEP_SIZE	256
#define HTTP_SESSION_HASH_MASK	(HTTP_SESSION_HASH_SIZE - 1)
#define GC_TIMER_INTERVAL	msecs_to_jiffies(1000)	/* 1s */
#define HTTP_SESSION_TIMEOUT	msecs_to_jiffies(5000)	/* 5s */

static int http_error_switch = 0;
static const int http_error_switch_min = 0;
static const int http_error_switch_max = 1;

int gc_timer_hash_index = 0;
static struct timer_list gc_timer;

struct http_session_table {
	spinlock_t  lock;
	struct hlist_head  head[HTTP_SESSION_HASH_SIZE];
};

struct http_session_key {
	uint32_t client_ip;
	uint16_t client_port;
	uint16_t res;
};

struct http_session {
	struct hlist_node hnode;
	atomic_t __refcnt;

	struct http_session_key key;

	uint32_t server_ip;
	uint32_t proxy_ip;
	uint16_t server_port;
	uint16_t proxy_port;

	struct synproxy_options syn_opt;
//	struct sk_buff *skb_http_get;
	struct sk_buff_head skb_list;
	uint32_t proxy_tsval;
	uint32_t jiff;			/*last access jiffies*/
}____cacheline_aligned_in_smp;

static struct http_session_table *http_sess_table;

static void dump_skb(const struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out)
{
	struct iphdr *iph;
	struct tcphdr *th;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;

	int i, llen, dlen;
	const unsigned char *pos;
	const int line_len = 16;
	char buff[1024];
	char *p;


	iph = ip_hdr(skb);

	p = buff;
	if (iph->protocol != IPPROTO_TCP){
		p += snprintf(p, sizeof(buff) - (p - buff), "%s: protocal not tcp [%d]\n", __FUNCTION__, iph->protocol);
		printk(KERN_DEBUG "%s", buff);
		return;
	}
	th = (struct tcphdr *) ((char*)iph + iph->ihl*4);
	dlen = ntohs(iph->tot_len) - iph->ihl * 4 - th->doff * 4;

	if (th->source == htons(80) || th->dest == htons(80) ||
		th->source == htons(8193) || th->dest == htons(8193)) {
		p += snprintf(p, sizeof(buff) - (p - buff),
			"%s:%lu skb len:%d, dlen:%d %pI4:%u(%s)-->%pI4:%u(%s), seq:%x, ack:%x, next seq:%x\n[",
			__FUNCTION__, jiffies, skb->len, dlen,
			&iph->saddr, ntohs(th->source), in  ? in->name  : "NULL",
			&iph->daddr, ntohs(th->dest),   out ? out->name : "NULL",
			ntohl(th->seq), ntohl(th->ack_seq), ntohl(th->seq)+(dlen > 0 ? dlen : 1) );
		if(th->fin) p += snprintf(p, sizeof(buff) - (p - buff), "F");
		if(th->syn) p += snprintf(p, sizeof(buff) - (p - buff), "S");
		if(th->rst) p += snprintf(p, sizeof(buff) - (p - buff), "R");
		if(th->ack) p += snprintf(p, sizeof(buff) - (p - buff), ".");
		if(th->psh) p += snprintf(p, sizeof(buff) - (p - buff), "P");
		if(th->urg) p += snprintf(p, sizeof(buff) - (p - buff), "U");
		if(th->ece) p += snprintf(p, sizeof(buff) - (p - buff), "E");
		if(th->cwr) p += snprintf(p, sizeof(buff) - (p - buff), "C");

		p += snprintf(p, sizeof(buff) - (p - buff), "] window[%04x] ", th->window);
		ct = nf_ct_get(skb, &ctinfo);
		if (ct){
			if((ct->status & IPS_NAT_MASK) == IPS_SRC_NAT)
				p += snprintf(p, sizeof(buff) - (p - buff), " [SNAT] ");
			if((ct->status & IPS_NAT_MASK) == IPS_DST_NAT)
				p += snprintf(p, sizeof(buff) - (p - buff), " [DNAT] ");
		}
		printk(KERN_DEBUG "%s", buff);
		p = buff;
		if(dlen >= HTTP_GET_MIN_LEN) {
			dlen = dlen > line_len * 4 ? line_len * 4 : dlen;
			pos = (char *)th + th->doff * 4;
			while (dlen) {
				llen = dlen > line_len ? line_len : dlen;
				p += snprintf(p, sizeof(buff) - (p - buff), "    ");
				for (i = 0; i < llen; i++)
					p += snprintf(p, sizeof(buff) - (p - buff), " %02x", pos[i]);
				for (i = llen; i < line_len; i++)
					p += snprintf(p, sizeof(buff) - (p - buff), "   ");
				p += snprintf(p, sizeof(buff) - (p - buff), "   ");
				for (i = 0; i < llen; i++) {
					if (isprint(pos[i]))
						p += snprintf(p, sizeof(buff) - (p - buff), "%c", pos[i]);
					else
						p += snprintf(p, sizeof(buff) - (p - buff), "*");
				}
				for (i = llen; i < line_len; i++)
					p += snprintf(p, sizeof(buff) - (p - buff), " ");
				printk(KERN_DEBUG "%s\n", buff);
				p = buff;
				pos += llen;
				dlen -= llen;
			}
		}
	}
}


uint32_t http_session_hash(struct http_session_key *key)
{
	uint32_t hash = key->client_ip + key->client_port + (key->client_port >> 1);

	return (hash % HTTP_SESSION_HASH_MASK);
}

int http_session_cmp(struct http_session_key *key1, struct http_session_key *key2)
{
	return memcmp(key1, key2, sizeof(struct http_session_key));
}

struct http_session* __http_session_search(struct http_session_key *key)
{
	struct hlist_node *pos;
	struct http_session *http_sess;
	uint32_t hash = http_session_hash(key);
	struct hlist_head *head = &http_sess_table->head[hash];

	hlist_for_each_entry(http_sess, pos, head, hnode) {
		DEBUG_PRINT(KERN_DEBUG "%s:  hash:%d, head:%p, http_sess:%p, pos:%p\n",
			__func__, hash, head, http_sess, pos);
		if (http_session_cmp(&http_sess->key, key) == 0) {
			DEBUG_PRINT(KERN_DEBUG "%s: [FIND %pI4:%u] hash:%d, head:%p, http_sess:%p, pos:%p\n",
				__func__, &key->client_ip, ntohs(key->client_port), hash, head, http_sess, pos);
			return http_sess;
		}
	}
	return NULL;
}

int __http_session_insert(struct http_session *http_sess)
{
	uint32_t hash = http_session_hash(&http_sess->key);
	struct hlist_head *head = &http_sess_table->head[hash];
	DEBUG_PRINT(KERN_DEBUG "%s:  hash:%d, head:%p, http_sess:%p\n",
		__func__, hash, head, http_sess);
	hlist_add_head(&http_sess->hnode, head);

	return 0;
}

void __http_session_del(struct http_session *http_sess)
{
	hlist_del(&http_sess->hnode);
	return;
}

void __http_session_free(struct http_session *http_sess)
{
	skb_queue_purge(&http_sess->skb_list);
	kfree(http_sess);
}

#define HTTP_SESS_TABLE_LOCK()	spin_lock_bh(&http_sess_table->lock)
#define HTTP_SESS_TABLE_UNLOCK() spin_unlock_bh(&http_sess_table->lock)

void put_http_session(struct http_session *http_sess)
{
	http_sess->jiff = jiffies;
	return atomic_dec(&http_sess->__refcnt);
}

void get_http_session(struct http_session *http_sess)
{
	return atomic_inc(&http_sess->__refcnt);
}

struct http_session* http_session_search(struct http_session_key *key)
{
	struct http_session* http_sess;
	HTTP_SESS_TABLE_LOCK();
	http_sess = __http_session_search(key);
	if (http_sess)
		get_http_session(http_sess);
	HTTP_SESS_TABLE_UNLOCK();

	return http_sess;
}

struct http_session* http_session_create(struct http_session_key *key)
{
	struct http_session* http_sess;

	http_sess = kzalloc(sizeof(struct http_session), GFP_ATOMIC);
	if (http_sess == NULL) {
		printk(KERN_INFO "%s: zalloc failed key:%p, client: %pI4,%u\n",
			__func__, key, &key->client_ip, key->client_port);
		return NULL;
	}
	memcpy(&http_sess->key, key, sizeof(struct http_session_key));
	skb_queue_head_init(&http_sess->skb_list);
	atomic_set((&http_sess->__refcnt), 1);


	HTTP_SESS_TABLE_LOCK();
	if (__http_session_search(&http_sess->key)) {
		HTTP_SESS_TABLE_UNLOCK();
		printk(KERN_INFO "%s: duplicated key:%p, client: %pI4,%u\n",
			__func__, key, &key->client_ip, ntohs(key->client_port));
		kfree(http_sess);
		http_sess = NULL;
	}
	else {
		__http_session_insert(http_sess);
		HTTP_SESS_TABLE_UNLOCK();
	}

	return http_sess;
}

struct http_session* http_session_del(struct http_session_key *key)
{
	struct http_session* http_sess;

	HTTP_SESS_TABLE_LOCK();
	http_sess = __http_session_search(key);
	if (http_sess)
		__http_session_del(http_sess);
	HTTP_SESS_TABLE_UNLOCK();

	return http_sess;
}

/* Packet is received from loopback */
static inline bool nf_is_loopback_packet(const struct sk_buff *skb)
{
	return skb->dev && skb->dev->flags & IFF_LOOPBACK;
}

/* stolen from kernel tcp_parse_options */
void http_error_tcp_parse_options(const struct sk_buff *skb,
		struct synproxy_options *opt_rx, struct tcphdr *th)
{
	uint8_t *ptr;
	int length = (th->doff * 4) - sizeof(struct tcphdr);

	ptr = (unsigned char *)(th + 1);

	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
			case TCPOPT_EOL:
				return;
			case TCPOPT_NOP:        /* Ref: RFC 793 section 3.1 */
				length--;
				continue;
			default:
				opsize = *ptr++;
				if (opsize < 2) /* "silly options" */
					return;
				if (opsize > length)
					return; /* don't parse partial options */
				switch (opcode) {
					case TCPOPT_TIMESTAMP:
						if (opsize == TCPOLEN_TIMESTAMP) {
							#ifndef __HTTP_ERROR_DISABLE_TS__
							opt_rx->options |= XT_SYNPROXY_OPT_TIMESTAMP;
							opt_rx->tsval = get_unaligned_be32(ptr);
							opt_rx->tsecr = get_unaligned_be32(ptr + 4);
							DEBUG_PRINT(KERN_DEBUG "%s:  skb:%p, rcv_tsval:%u, rcv_tsecr:%u\n",
							__func__, skb, opt_rx->tsval, opt_rx->tsecr);
							#endif
						}
						break;
#if 0
					/* http_error could not work with TCP MD5*/
					case TCPOPT_MD5SIG:
					/* TODO: items are NOT supportted by 2013-03-12. Junwei*/
					case TCPOPT_MSS:
						if (opsize == TCPOLEN_MSS && th->syn && !estab) {
							u16 in_mss = get_unaligned_be16(ptr);
							if (in_mss) {
								if (opt_rx->user_mss &&
										opt_rx->user_mss < in_mss)
									in_mss = opt_rx->user_mss;
								opt_rx->mss_clamp = in_mss;
							}
						}
						break;
					case TCPOPT_EXP:
					case TCPOPT_SACK_PERM:
					case TCPOPT_SACK:
#endif
				}
				ptr += opsize-2;
				length -= opsize;
		}
	}
}

/* stolen from kernel tcp_parse_options */
uint8_t* http_error_tcp_get_timestamp_option(const struct sk_buff *skb,
	struct tcphdr *th)
{
	uint8_t *ptr;
	int length = (th->doff * 4) - sizeof(struct tcphdr);

	ptr = (uint8_t *)(th + 1);

	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
			case TCPOPT_EOL:
				return NULL;
			case TCPOPT_NOP:        /* Ref: RFC 793 section 3.1 */
				length--;
				continue;
			default:
				opsize = *ptr++;
				if (opsize < 2) /* "silly options" */
					return NULL;
				if (opsize > length)
					return NULL; /* don't parse partial options */
				switch (opcode) {
					case TCPOPT_TIMESTAMP:
						if (opsize == TCPOLEN_TIMESTAMP)
						    return ptr;
						break;
				}
				ptr += opsize-2;
				length -= opsize;
		}
	}

	return NULL;
}

static void
synproxy_build_options(struct tcphdr *th, const struct synproxy_options *opts)
{
	__be32 *ptr = (__be32 *)(th + 1);
	u8 options = opts->options;

	if (options & XT_SYNPROXY_OPT_MSS)
		*ptr++ = htonl((TCPOPT_MSS << 24) |
				(TCPOLEN_MSS << 16) |
				opts->mss);

	if (options & XT_SYNPROXY_OPT_TIMESTAMP) {
		if (options & XT_SYNPROXY_OPT_SACK_PERM)
			*ptr++ = htonl((TCPOPT_SACK_PERM << 24) |
					(TCPOLEN_SACK_PERM << 16) |
					(TCPOPT_TIMESTAMP << 8) |
					TCPOLEN_TIMESTAMP);
		else
			*ptr++ = htonl((TCPOPT_NOP << 24) |
					(TCPOPT_NOP << 16) |
					(TCPOPT_TIMESTAMP << 8) |
					TCPOLEN_TIMESTAMP);

		*ptr++ = htonl(opts->tsval);
		*ptr++ = htonl(opts->tsecr);
	} else if (options & XT_SYNPROXY_OPT_SACK_PERM)
		*ptr++ = htonl((TCPOPT_NOP << 24) |
				(TCPOPT_NOP << 16) |
				(TCPOPT_SACK_PERM << 8) |
				TCPOLEN_SACK_PERM);

	if (options & XT_SYNPROXY_OPT_WSCALE)
		*ptr++ = htonl((TCPOPT_NOP << 24) |
				(TCPOPT_WINDOW << 16) |
				(TCPOLEN_WINDOW << 8) |
				opts->wscale);
}

static struct iphdr *
http_error_build_ip(struct sk_buff *skb, u32 saddr, u32 daddr)
{
	struct iphdr *iph;

	skb_reset_network_header(skb);
	iph = (struct iphdr *)skb_put(skb, sizeof(*iph));
	iph->version    = 4;
	iph->ihl        = sizeof(*iph) / 4;
	iph->tos        = 0;
	iph->id         = 0;
	iph->frag_off   = htons(IP_DF);
	iph->ttl        = 60;
	iph->protocol   = IPPROTO_TCP;
	iph->check      = 0;
	iph->saddr      = saddr;
	iph->daddr      = daddr;

	return iph;
}

static struct sk_buff*
http_error_fake_syn(struct sk_buff *orig_skb, const struct tcphdr *th,
		struct http_session *http_sess)
{
	struct sk_buff *nskb;
	struct iphdr *niph;
	struct tcphdr *nth;
	unsigned int tcp_hdr_size;
	uint8_t options = http_sess->syn_opt.options;
	struct synproxy_options syn_opt = http_sess->syn_opt;

	syn_opt.tsval--;
	tcp_hdr_size = sizeof(*nth) + MAX_TCP_OPTION_SPACE;
	nskb = alloc_skb(sizeof(*niph) + tcp_hdr_size + MAX_TCP_HEADER + sizeof(struct ethhdr),
			GFP_ATOMIC);
	if (nskb == NULL)
		return NULL;
	skb_reserve(nskb, MAX_TCP_HEADER);

	niph = http_error_build_ip(nskb, http_sess->key.client_ip, http_sess->proxy_ip);

	skb_set_transport_header(nskb, sizeof(niph->ihl*4));
	nth = (struct tcphdr *)skb_put(nskb, tcp_hdr_size);
	nth->source     = th->source;
	nth->dest       = http_sess->proxy_port;
	nth->seq        = htonl(ntohl(th->seq) - 1);
	nth->ack_seq    = htonl(ntohl(th->ack_seq) - 1);
	tcp_flag_word(nth) = TCP_FLAG_SYN;
	if (options & XT_SYNPROXY_OPT_ECN)
		tcp_flag_word(nth) |= TCP_FLAG_ECE | TCP_FLAG_CWR;

	nth->doff       = tcp_hdr_size / 4;
	nth->window     = th->window;
	nth->check      = 0;
	nth->urg_ptr    = 0;
	synproxy_build_options(nth, &syn_opt);

	nth->check = tcp_v4_check(nskb->len - sizeof(struct iphdr), niph->saddr, niph->daddr,
			csum_partial(nth, nth->doff << 2, 0));

	/*IP total length and checksum */
	nskb->ip_summed = CHECKSUM_NONE;
	niph->tot_len = htons(sizeof(struct iphdr) + tcp_hdr_size);
	niph->check = 0;
	niph->check = ip_fast_csum((unsigned char *)niph, niph->ihl);

	/*Prepare to enter network statck*/
	nskb->dev = orig_skb->dev;
	nskb->protocol = ntohs(ETH_P_IP);
	nskb->pkt_type = PACKET_HOST;
	nskb->ip_summed = CHECKSUM_UNNECESSARY;
	DEBUG_PRINT(KERN_DEBUG "%s:  nskb:%p, nskb len:%u, %pI4(%u) ->%pI4(%u) TCP seq:%x, ack:%x\n",
		       	__func__, nskb, nskb->len, &niph->saddr,
			ntohs(nth->source), &niph->daddr, ntohs(nth->dest),
			ntohl(nth->seq), ntohl(nth->ack_seq));

	return nskb;
}

static uint16_t http_error_get_opt_mss(struct sk_buff *skb, struct iphdr *iph, struct tcphdr *th)
{
    uint8_t *op;
    uint32_t i;
    uint32_t oplen = th->doff*4 - sizeof(struct tcphdr);
    if(oplen == 0)
        return 0;

    op = ((uint8_t*)th + sizeof(struct tcphdr));
    for(i=0;i<oplen;){
        if(op[i] == TCPOPT_MSS && th->doff*4 - i >= TCPOLEN_MSS && op[i+1] == TCPOLEN_MSS)
        {
            return ntohs(*(uint16_t*)&(op[i+2]));
        }

        if(op[i] < 2) //no-op
            i++;
        else
            i += op[i+1];
    }
    return 0;

}

static void rm_ts(struct sk_buff *skb, struct tcphdr *th, struct iphdr *iph)
{
	uint8_t *ptr, *base;
	int i;
	int length = (th->doff*4) - sizeof(struct tcphdr);
	ptr = (unsigned char *)(th + 1);
	while (length > 0){
		int opsize;
		int opcode = *ptr++;
		switch (opcode) {
		case TCPOPT_EOL:
			return;
		case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2) /* "silly options" */
				return;
			if (opsize > length)
				return; /* don't parse partial options */
			switch (opcode) {
			case TCPOPT_TIMESTAMP:

				if (opsize == TCPOLEN_TIMESTAMP) {
					DEBUG_PRINT(KERN_DEBUG "rm_ts begin\n");
					base = ptr - 2;
					for (i=0; i<opsize; i++, base++){
						*base = TCPOPT_NOP;
					}
					th->check = 0;
					th->check = tcp_v4_check(skb->len - sizeof(struct iphdr), iph->saddr, iph->daddr,
							csum_partial(th, th->doff << 2, 0));
					iph->check = 0;
					iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
					return;
				}
			}
			ptr += opsize-2;
			length -= opsize;
		}
	}
}

/* 
 */
static unsigned int ipv4_http_error_pre_hook(unsigned int hooknum,
	struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	unsigned int ret = NF_ACCEPT;
	struct sk_buff *new_skb;
	struct iphdr *iph;
	struct tcphdr *th;
#ifdef __HTTP_ERROR_DEBUG__
	unsigned char *http;
	int offset;
#endif
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	int dlen = 0;
	const char * data = NULL;
	uint32_t sig = 0;
	int match;
	int action;

	struct http_session_key key;
	struct http_session *session;

	DEBUG_PRINT(KERN_DEBUG "prerouting\n");
	DUMP_SKB(skb, in, out);

//	if (!http_error_switch)
		return NF_ACCEPT;
#if 0
	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL){
		return NF_ACCEPT;
	}

	if (ct->tproxy.http_error_flags & HTTP_ERROR_F_IGNORE)
		return NF_ACCEPT;

	if (nf_is_loopback_packet(skb))
		goto http_error_ignore;

	iph = ip_hdr(skb);
	if (iph->protocol != IPPROTO_TCP)
		goto http_error_ignore;

	th = (struct tcphdr *) ((char*)iph + iph->ihl*4);
	dlen = ntohs(iph->tot_len) - iph->ihl * 4 - th->doff * 4;

#ifdef __HTTP_ERROR_DISABLE_TS__
	if (th->syn && (th->dest == htons(80) || th->source == htons(80)))
		rm_ts(skb, th, iph);
#endif

	if (th->source == htons(80)) {
		ct->tproxy.bytes_of_flow += dlen;
		DEBUG_PRINT(KERN_DEBUG "%s, sport==80, bytes=%ld, nums=%d\n", __FUNCTION__, ct->tproxy.bytes_of_flow, ct->tproxy.nums_of_get_packet);
		return NF_ACCEPT;
	}
	if (th->dest != htons(80))
		goto http_error_ignore;

	if (!th->syn && (ct->status & IPS_NAT_MASK) != IPS_SRC_NAT)
		return NF_ACCEPT;

	//get MSS from syn package
	if (th->syn && !th->ack) {
		ct->tproxy.http_error_mss = http_error_get_opt_mss(skb, iph, th);
	}

	/* todo: check HTTP_ERROR_F_KEEP(unlikely) */
	if (ct->tproxy.http_error_flags & HTTP_ERROR_F_KEEP){
		DEBUG_PRINT(KERN_DEBUG "%s,ct->tproxy.http_error_flags & HTTP_ERROR_F_KEEP\n", __FUNCTION__);
		key.client_ip = iph->saddr;
		key.client_port = th->source;
		key.res = 0;
		session = http_session_search(&key);
		if (session == NULL){
			/* there is something wrong  */
			DEBUG_PRINT(KERN_DEBUG "%s,no seession found!!!\n", __FUNCTION__);
			ct->tproxy.http_error_flags &= ~HTTP_ERROR_F_KEEP;
			return NF_ACCEPT;
		}
		ct->tproxy.bytes_of_flow += dlen;
		ct->tproxy.nums_of_get_packet += 1;
		DEBUG_PRINT(KERN_DEBUG "skb_queue_tail[%p]\n", skb);
		skb_queue_tail(&session->skb_list, skb);
		put_http_session(session);
		return NF_STOLEN;
	}

	/* skip download flow */
	if ((0 == ct->tproxy.nums_of_get_packet && ct->tproxy.bytes_of_flow >= FLOW_LIMIT)
		|| (1 == ct->tproxy.nums_of_get_packet && ct->tproxy.bytes_of_flow >= FLOW_LIMIT)) {
		goto http_error_ignore;
	}

	/* incase non-care tcp flow < 18 bytes */
	if (dlen < HTTP_GET_MIN_LEN) {
		return NF_ACCEPT;
	}

	data = (void *)th + (th->doff * 4);
	sig = *(uint32_t *)data;

	/* accept but don't care "POST" request */
	if (sig == constant_post_string) {
		return NF_ACCEPT;
	} else if (sig == constant_get_string) {
		ct->tproxy.bytes_of_flow += dlen;
		ct->tproxy.nums_of_get_packet += 1;
		DEBUG_PRINT(KERN_DEBUG "%s, GET request, bytes=%ld, nums=%d\n", __FUNCTION__, ct->tproxy.bytes_of_flow, ct->tproxy.nums_of_get_packet);
	} else {
		ct->tproxy.bytes_of_flow += dlen;
		DEBUG_PRINT(KERN_DEBUG "%s, unkonwn request, bytes=%ld, nums=%d\n", __FUNCTION__, ct->tproxy.bytes_of_flow, ct->tproxy.nums_of_get_packet);
		return NF_ACCEPT;
	}

	PERF_DEBUG(KERN_DEBUG "%s, begin match at:%lu;", __FUNCTION__, (jiffies));
	match = web_filter_match(skb);
	PERF_DEBUG(KERN_DEBUG "%s, end   match at:%lu;", __FUNCTION__, (jiffies));

	action = GET_ACTION_KIND_1_VAL(match);

	/* hook GET request to local proxy.*/
	if (action && action1_vec[action].ip && action1_vec[action].port) {
		key.client_ip = iph->saddr;
		key.client_port = th->source;
		key.res = 0;

		DEBUG_PRINT("[*]start to create session at: %u, key: [%u:%d]\n.",jiffies,key.client_ip,key.client_port);
		session = http_session_create(&key);
		ct->tproxy.http_error_flags |= HTTP_ERROR_F_KEEP;
		if (session == NULL) {
			printk(KERN_DEBUG "%s: Failed create tcp session, time:%lu, skb:%p, len:%d,  %pI4:%u-->%pI4:%u\n",
				__func__, jiffies, skb, skb->len,
				&iph->saddr, ntohs(th->source), &iph->daddr, ntohs(th->dest));
			goto next_action;
		}
		DEBUG_PRINT(KERN_DEBUG "%s: [INS tcp sess] %pI4:%u --> %pI4:%u\n", __func__, &iph->saddr, ntohs(th->source), &iph->daddr, ntohs(th->dest));

		session->server_ip = iph->daddr;
		session->server_port = th->dest;

		session->proxy_ip = action1_vec[action].ip;
		session->proxy_port = action1_vec[action].port;

		//BUG_ON(session->skb_http_get);
		session->syn_opt.wscale = ct->proto.tcp.seen[0].td_scale;
		if (session->syn_opt.wscale > 0)
			session->syn_opt.options |= XT_SYNPROXY_OPT_WSCALE;
		http_error_tcp_parse_options(skb, &session->syn_opt, th);

		//carry on MSS, 1460 hardcoded
		session->syn_opt.options |= XT_SYNPROXY_OPT_MSS;
		session->syn_opt.mss = ct->tproxy.http_error_mss;

#ifdef __HTTP_ERROR_DEBUG__
		http = (unsigned char *)th + th->doff*4;
		offset = http - (unsigned char*)iph;

		DEBUG_PRINT(KERN_DEBUG "%s: [HTTP GET] dev:%s, skb len:%d, data_len:%d %pI4:%u-->%pI4:%u, seq:%x, ack:%x, http len:%d\n\n",
			__func__, in ? in->name : "NULL", skb->len, skb->data_len, &iph->saddr,
			ntohs(th->source), &iph->daddr, ntohs(th->dest),
			ntohl(th->seq), ntohl(th->ack_seq), skb->len - offset);
#endif

		new_skb = http_error_fake_syn(skb, th, session);
		if (new_skb) {
			if (netif_rx(new_skb) == NET_RX_SUCCESS) {
				ret = NF_STOLEN;
				DEBUG_PRINT(KERN_DEBUG "skb_queue_head[%p]\n", skb);
				//session->skb_http_get = skb;
				skb_queue_head(&session->skb_list, skb);
			} else {
				printk(KERN_DEBUG "netif_rx failed 2\n");
				kfree_skb(new_skb);
			}
		}
		put_http_session(session);
		PERF_DEBUG(KERN_DEBUG "%s, end fake SYN at:%lu;", __FUNCTION__, (jiffies));
	}

next_action:
	action = GET_ACTION_KIND_2_VAL(match);
	/* broadcast the orignal HTTP packet to userspace. */
	if (action)
		mi_netlink_broadcast(skb);

	return ret;

http_error_ignore:
	ct->tproxy.http_error_flags |= HTTP_ERROR_F_IGNORE;

#ifdef HNDCTF
	//clear ctf mark, ignore this flow
	ct->ctf_flags &= ~CTF_FLAGS_EXCLUDED;
	//DEBUG_PRINT(KERN_DEBUG "Add CTF, bytes=%ld, nums=%d\n", ct->tproxy.bytes_of_flow, ct->tproxy.nums_of_get_packet);
#endif

#endif

	return ret;
}

static int http_error_replace_tsecr(struct sk_buff *skb, uint32_t ts_val)
{
	struct iphdr *iph;
	struct tcphdr *th;
	uint8_t *ptr;
	uint32_t old_ts;

	/* Fake match: hook http request from internl network to wlan */
	iph = ip_hdr(skb);
	BUG_ON(((unsigned char *)iph) != skb->data);
	if (iph->protocol != IPPROTO_TCP)
		return -EINVAL;

	th = (struct tcphdr *) ((char*)iph + iph->ihl*4);
	ptr = http_error_tcp_get_timestamp_option(skb, th);
	if (ptr == NULL)
		return -EINVAL;

	ptr += 4;
	old_ts = get_unaligned_be32(ptr);
	put_unaligned_le32(ts_val, ptr);
	//inet_proto_csum_replace4(&th->check, skb, old_ts, ts_val, 0);
	DEBUG_PRINT(KERN_DEBUG "%s: timestamp ts_val: %u-->%u\n", __func__, old_ts, ts_val);

	return 0;
}

static unsigned int http_error_skb_queue_send(struct http_session * http_sess)
{
	struct nf_conn *ct = NULL;
	enum ip_conntrack_info ctinfo;
	struct sk_buff * resend_skb = NULL;
	do {
		resend_skb = skb_dequeue(&http_sess->skb_list);
		if(resend_skb) {
			DEBUG_PRINT(KERN_DEBUG "%s: resend http skb:[%p] at %u.\n", __func__, resend_skb, jiffies);
			ct = nf_ct_get(resend_skb, &ctinfo);
			if (!ct) {
				printk("%s, impossible, ct should be valid!!!\n", __func__);
				kfree_skb(resend_skb);
				continue;
			}
			nf_ct_put(ct);
			resend_skb->nfct = NULL;

			// for now, we don't use timestamp
			if (http_sess->proxy_tsval) {
				http_error_replace_tsecr(resend_skb, http_sess->proxy_tsval);
				resend_skb->ip_summed = CHECKSUM_UNNECESSARY;
			}
			if (netif_rx(resend_skb) == NET_RX_SUCCESS){
				continue;
			} else {
				printk(KERN_DEBUG "netif_rx failed\n");
				kfree_skb(resend_skb);
			}
		}
	} while(resend_skb != NULL);
	return 0;
}

static unsigned int ipv4_http_error_post_hook(unsigned int hooknum,
	struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	int ret = NF_ACCEPT;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	struct iphdr *iph;
	struct tcphdr *th;
	struct http_session_key key;
	struct http_session *http_sess;
	int proxy_resp = 0;


	DEBUG_PRINT(KERN_DEBUG "postrouting\n");
	DUMP_SKB(skb, in, out);

//	if (!http_error_switch)
		return NF_ACCEPT;
#if 0
	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return NF_ACCEPT;

	if (nf_is_loopback_packet(skb))
		return NF_ACCEPT;

	/* Fake match: hook http request from internl network to wlan */
	iph = ip_hdr(skb);
	BUG_ON(((unsigned char *)iph) != skb->data);
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	th = (struct tcphdr *) ((char*)iph + iph->ihl*4);
	if (th->syn && th->ack) {
		proxy_resp = SYN_ACK;
	} else if (th->rst && th->ack) {
		proxy_resp = RST_ACK;
	} else {
		return NF_ACCEPT;
	}

	key.client_ip = iph->daddr;
	key.client_port = th->dest;
	key.res = 0;

	http_sess = http_session_search(&key);
	if (http_sess == NULL)
		return NF_ACCEPT;

	DEBUG_PRINT(KERN_DEBUG "%s: [FIND tcp sess] %pI4:%u --> %pI4:%u\n",
			__func__, &iph->saddr, ntohs(th->source), &key.client_ip, ntohs(key.client_port));

	if (th->source == http_sess->proxy_port && iph->saddr == http_sess->proxy_ip) {
		struct synproxy_options tmp_opt;
		struct sk_buff * resend_skb = skb_peek(&http_sess->skb_list);
		if (!resend_skb) {
			goto put_http_sess;
		}
		DEBUG_PRINT(KERN_DEBUG "%s: skb_peek get skb:[%p] at %u.\n", __func__, resend_skb, jiffies);

		ct = nf_ct_get(resend_skb, &ctinfo);
		if (!ct) {
			goto put_http_sess;
		}
		switch (proxy_resp)
		{
			case SYN_ACK:
				DEBUG_PRINT("SYN ACK from %pI4:%u\n", &iph->saddr, ntohs(th->source));
				http_error_tcp_parse_options(skb, &tmp_opt, th);
				http_sess->proxy_tsval = tmp_opt.tsval;

				DEBUG_PRINT(KERN_DEBUG "%s: time:%lu, SYN/ACK skb:%p, dev:%s, skb len:%d, %pI4:%u-->%pI4:%u\n",
						__func__, jiffies, skb, in ? in->name : "NULL", skb->len, &iph->saddr,
						ntohs(th->source), &iph->daddr, ntohs(th->dest));
				DEBUG_PRINT(KERN_DEBUG "%s: tmp_opt:%u,%u, http ts opt:%u,%u\n",
						__func__, tmp_opt.tsval, tmp_opt.tsecr,
						http_sess->syn_opt.tsval, http_sess->syn_opt.tsecr);
				DEBUG_PRINT(KERN_DEBUG "%s: IPS_DYING:%x, IPS_NAT_MASK:%x ct->status:%x\n",
						__func__, ct->status & IPS_DYING, ct->status & IPS_NAT_MASK, ct->status);

				if (!(ct->status & IPS_DYING) && (ct->status & IPS_NAT_MASK) == IPS_SRC_NAT) {
					spin_lock_bh(&nf_conntrack_lock);
					hlist_nulls_del_rcu(&ct->tuplehash[IP_CT_DIR_REPLY].hnnode);
					hlist_nulls_del_rcu(&ct->tuplehash[IP_CT_DIR_ORIGINAL].hnnode);
					ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip = http_sess->proxy_ip;
					ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.tcp.port = http_sess->proxy_port;
					ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip = http_sess->key.client_ip;
					ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.tcp.port = http_sess->key.client_port;
					ct->status &= ~IPS_SRC_NAT;
					ct->status |= IPS_DST_NAT;
					nf_conntrack_hash_insert(ct);
					ct->tproxy.http_error_flags &= ~HTTP_ERROR_F_KEEP;
					spin_unlock_bh(&nf_conntrack_lock);

					http_error_skb_queue_send(http_sess);
					kfree_skb(skb); //todo: free out of softirq?
					ret = NF_STOLEN;
					PERF_DEBUG("%s, end fake Get at:%lu;", __FUNCTION__, (jiffies));
				}
				break;
			case RST_ACK:
				DEBUG_PRINT("RST ACK from %pI4:%u\n", &iph->saddr, ntohs(th->source));
				// ignore this session
				if (!(ct->status & IPS_DYING) && (ct->status & IPS_NAT_MASK) == IPS_SRC_NAT) {
					spin_lock_bh(&nf_conntrack_lock);
					ct->tproxy.http_error_flags &= ~HTTP_ERROR_F_KEEP;
					ct->tproxy.http_error_flags |= HTTP_ERROR_F_IGNORE;
					spin_unlock_bh(&nf_conntrack_lock);

					http_error_skb_queue_send(http_sess);
					kfree_skb(skb); //todo: free out of softirq?
					ret = NF_STOLEN;
				}
				break;
			default:
				printk("%s, impossible here!\n", __func__);
				break;
		}

// put http_sess then free it
put_http_sess:
		put_http_session(http_sess);

		HTTP_SESS_TABLE_LOCK();
		__http_session_del(http_sess);
		HTTP_SESS_TABLE_UNLOCK();
		__http_session_free(http_sess);
		PERF_DEBUG("%s, end fake END at:%lu;", __FUNCTION__, (jiffies));
		return ret;
	} else {
		DEBUG_PRINT(KERN_DEBUG "%s, NOT from proxy, ACCEPT\n", __func__);
		put_http_session(http_sess);
		return NF_ACCEPT;
	}
#endif
}

static struct nf_hook_ops ipv4_synproxy_ops[] __read_mostly = {
	/* hook 'http GET'*/
	{
		.hook           = ipv4_http_error_pre_hook,
		.owner          = THIS_MODULE,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_PRE_ROUTING,
		.priority       = NF_IP_PRI_CONNTRACK_CONFIRM - 1,
	},
	/* hook 'syn/ack from proxy for faked syn' to send 'ack'*/
	{
		.hook           = ipv4_http_error_post_hook,
		.owner          = THIS_MODULE,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_POST_ROUTING, //todo why not locatout?
		.priority       = NF_IP_PRI_CONNTRACK_CONFIRM,
	},
};

#if 0
//cmd example: add 1 192.168.1.1 65534
//cmd example: del 1
int http_error_run_action_cmd(char *cmd) {
	char add_cmd_prefix[] = "add ";
	char del_cmd_prefix[] = "del ";
	int prefix_size = strlen(add_cmd_prefix);
	int index;

	struct action_1_info tmp;

	if (strlen(cmd) <= 0)
		return -EINVAL;

	if (strncasecmp(add_cmd_prefix, cmd, prefix_size) == 0) {
		cmd += strlen(add_cmd_prefix);
	        sscanf(cmd, "%d %d.%d.%d.%d %d",
			&index, &tmp.byte[0], &tmp.byte[1],
			&tmp.byte[2], &tmp.byte[3], &tmp.port);
		if (index <= 0 || index >= ACTION_SIZE)
			return -EINVAL;

		tmp.port = htons(tmp.port);
		memcpy(&action1_vec[index], &tmp, sizeof(tmp));
	}
	else if (strncasecmp(del_cmd_prefix, cmd, prefix_size) == 0) {
		cmd += strlen(del_cmd_prefix);
	        sscanf(cmd, "%d", &index);
		if (index <= 0 || index >= ACTION_SIZE)
			return -EINVAL;
		memset(&action1_vec[index], 0, sizeof(action1_vec[index]));
	}
	else {
		printk(KERN_DEBUG "Wrong format !! usage:%s\n",
			"add 1 192.168.1.1 65534 or del 1");
		return -EINVAL;
	}
	return 0;
}

int proc_http_error_action_table_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int i;
	int len;
	int total;
	char __user *p;
	char c;
	const int str_max_size = 32; //ADD 1 192.168.1.1 65534
	char vec_str[str_max_size * ACTION_SIZE]; //no more than 256..

	if (write) {
		if (!*lenp)
			return 0;
		len = 0;
		p = buffer;
		while (len < *lenp) {
			if (get_user(c, p++))
				return -EFAULT;
			if (c == 0 || c == '\n')
				break;
			len++;
			if (len >= str_max_size)
				return -EINVAL;
		}
		if (len >= str_max_size)
			return -EINVAL;
		if(copy_from_user(vec_str, buffer, len))
			return -EFAULT;
		vec_str[len] = 0;
		http_error_run_action_cmd(vec_str);
	} else {
		/*todo: if ACTION_SIZE > 8 */
		if (*ppos > 0) {
			*lenp = 0;
			return 0;
		};

		*lenp = 0;
		len = 0;
		for (i = 0; i < ACTION_SIZE; i++)
			len += sprintf(vec_str + len, "vec[%d] %pI4:%d\n",
				i, &action1_vec[i].ip, ntohs(action1_vec[i].port));

		total = copy_to_user(buffer, vec_str, len);
		if (total) {
			printk(KERN_DEBUG "vec_str is too long(%d), only %d bytes copied\n",
				len, total);
			len = 0;
		}
		*lenp = len;
		*ppos += len;
	}
	return 0;
}

struct ctl_table http_error_action_table[] = {
	{
		.procname = "http_error_action",
		.data = NULL,
		.maxlen = 1024,
		.mode = 0644,
		.proc_handler = proc_http_error_action_table_handler,
	},
	{
		.procname = "http_error_switch",
		.data = &http_error_switch,
		.maxlen = sizeof(unsigned int),
		.mode = 0644,
		.proc_handler = proc_dointvec_minmax,
		.extra1 = &http_error_switch_min,
		.extra2 = &http_error_switch_max,
	},
	{}
};

int http_error_sysfs_init(void)
{
	memset(action1_vec, 0, sizeof(action1_vec));
	http_error_action_table_header = 
		register_sysctl_paths(net_ipv4_ctl_path, http_error_action_table);
	if (http_error_action_table_header == NULL)
		return -EPERM;
	return 0;
}

void http_error_sysfs_exit(void)
{
	if (http_error_action_table_header)
		unregister_sysctl_table(http_error_action_table_header);
	return;
}

#endif

void gc_timer_callback(unsigned long data)
{
	int i;
	struct hlist_node *pos;
	struct hlist_node *n;
	struct http_session *http_sess;
	struct hlist_head *head;

	for (i=0; i<HTTP_SESSION_STEP_SIZE; i++) {
		head = &http_sess_table->head[gc_timer_hash_index];
		HTTP_SESS_TABLE_LOCK();
		hlist_for_each_entry_safe(http_sess, pos, n, head, hnode) {
			if (time_after(jiffies, http_sess->jiff + HTTP_SESSION_TIMEOUT)
					&& atomic_read(&http_sess->__refcnt) == 0) {
				__http_session_del(http_sess);
				__http_session_free(http_sess);
			}
		}
		HTTP_SESS_TABLE_UNLOCK();

		gc_timer_hash_index++;
		gc_timer_hash_index &= HTTP_SESSION_HASH_MASK;
	}
	mod_timer(&gc_timer, jiffies + GC_TIMER_INTERVAL);

	return;
}

static int __init http_error_init(void)
{
	int i = 0;
	int err = 0;

/*
	// init match
	err = init_webfilter_match();
	if (err)
		return err;

	err = mi_netlink_init();
	if (err)
		goto err_netlink;
*/

	http_sess_table = kzalloc(sizeof(struct http_session_table), GFP_ATOMIC);
	if (NULL == http_sess_table) {
		err = -ENOMEM;
		goto err_malloc;
	}

	spin_lock_init(&http_sess_table->lock);
	for (i = 0; i < HTTP_SESSION_HASH_SIZE; i++) {
		INIT_HLIST_HEAD(&http_sess_table->head[i]);
	}

	err = nf_register_hooks(ipv4_synproxy_ops,
			ARRAY_SIZE(ipv4_synproxy_ops));
	if (err < 0) {
		printk(KERN_ERR"%s, register nf_hooks error!\n", __FUNCTION__);
		goto err_hook;
	}

	//http_error_sysfs_init();

	setup_timer(&gc_timer, gc_timer_callback, 0);
	mod_timer(&gc_timer, jiffies + GC_TIMER_INTERVAL);
	printk("%s, succeed!\n", __FUNCTION__);
	return 0;

err_hook:
	kfree(http_sess_table);
err_malloc:
    //mi_netlink_exit();
err_netlink:
	//fini_webfilter_match();
	return err;
}

static void __exit http_error_exit(void)
{
	int i;
	int ret;
	struct hlist_node *pos;
	struct hlist_node *n;
	struct http_session *http_sess;
	struct hlist_head *head;

	//http_error_sysfs_exit();
	nf_unregister_hooks(ipv4_synproxy_ops, ARRAY_SIZE(ipv4_synproxy_ops));

	ret = del_timer_sync(&gc_timer);
	if (ret)
		printk("Timer is still in use...\n");

	HTTP_SESS_TABLE_LOCK();
	for (i=0; i<HTTP_SESSION_HASH_SIZE; i++) {
		head = &http_sess_table->head[i];
		hlist_for_each_entry_safe(http_sess, pos, n, head, hnode) {
			__http_session_del(http_sess);
			__http_session_free(http_sess);
		}
	}
	HTTP_SESS_TABLE_UNLOCK();
	kfree(http_sess_table);
	//mi_netlink_exit();
	// clean match
	//fini_webfilter_match();
}

module_init(http_error_init);
module_exit(http_error_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yu Bo <yubo@xiaomi.com>");
MODULE_DESCRIPTION("http error replace");
