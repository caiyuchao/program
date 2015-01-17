/*
    web filter (experimental)
    http(web) request filter moudle
    Copyright (C) 2014 miwifi-network

    Licensed under GNU GPL v2 or later.
*/
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <linux/list.h>
#include <linux/hash.h>
#include <linux/types.h>
#include <crypto/hash.h>
#include <net/ip.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include "ipt_web_filter.h"

//#define __WEBFILTER_MATCH_MODULE__
#ifdef __WEBFILTER_MATCH_MODULE__
#include <linux/module.h>
#endif

#if 0 // per cpu data for target action
DEFINE_PER_CPU_ALIGNED(struct softnet_data, softnet_data);
EXPORT_PER_CPU_SYMBOL(softnet_data);
	for_each_possible_cpu(i) {
		struct softnet_data *sd = &per_cpu(softnet_data, i);
        memset(sd, 0, sizeof(*sd));
    }

// support get/reset interface
int get_web_target_action();
void reset_web_target_action();
#endif

/*===========================================*/

/* netlink for webfilter module */
#define NETLINK_WEB_FILTER (MAX_LINKS - 1)

/* md5 sum length of uri or host */
#define MD5_HASH_LEN 16

/* Http GET request head info */
#define WEBFILTER_GET_URL_OK    0x01
#define WEBFILTER_GET_REFER_OK  0x02
#define WEBFILTER_GET_ALL_OK    (WEBFILTER_GET_URL_OK | WEBFILTER_GET_REFER_OK)

/* <-- same as webfilter_config.h */
/* webfilter list type */
enum
{
    WEBFILTER_LIST_URL = 0,
    WEBFILTER_LIST_HOST,
};

/* webfilter element type */
enum
{
    WEBFILTER_ELEM_NONE = 0,
    WEBFILTER_ELEM_SECURITY = WEBFILTER_ELEM_NONE,    // security host
    WEBFILTER_ELEM_AD_HOST,         // AD host
    WEBFILTER_ELEM_AD_THIRD_PATRY,  // AD host, third-party
    WEBFILTER_ELEM_AD_URL,          // AD URL
    WEBFILTER_ELEM_MAX = 16,        // MAX, ushort bits
};

#define WEBFILTER_RULE_NONE     (0)
#define WEBFILTER_RULE_SECURITY (1<<WEBFILTER_ELEM_SECURITY)
#define WEBFILTER_RULE_AD_HOST  (1<<WEBFILTER_ELEM_AD_HOST)
#define WEBFILTER_RULE_AD_THIRD (1<<WEBFILTER_ELEM_AD_THIRD_PATRY)
#define WEBFILTER_RULE_AD_URL   (1<<WEBFILTER_ELEM_AD_URL)
#define WEBFILTER_RULE_AD   (WEBFILTER_RULE_AD_HOST | WEBFILTER_RULE_AD_THIRD | WEBFILTER_RULE_AD_URL)
#define WEBFILTER_RULE_MAX  (1<<WEBFILTER_ELEM_MAX - 1)

/* webfilter rule action type: add, del, purge */
enum
{
    WEBFILTER_TYPE_ADD = 0,
    WEBFILTER_TYPE_DEL,
    WEBFILTER_TYPE_PURGE,
    WEBFILTER_TYPE_PURGE_SPECIAL,
};

/* webfilter rule target type : drop, reject, redirect.. */
enum
{
    WEBFILTER_TARGET_DROP = 0,
    WEBFILTER_TARGET_REJECT,
    WEBFILTER_TARGET_REDIRECT,
};

/* webfilter string type : host or uri */
enum
{
    WEBFILTER_STR_URI = 0,
    WEBFILTER_STR_HOST,
    WEBFILTER_STR_REFER,
};

/* ---------------------------------> */

/* <-- same as webfilter_config.c --- */

/* the place of the msg stored */
enum
{
    NETLINK_MSG_FOLLOW = 0,
    NETLINK_MSG_MMAP,
};

#define LEN_WF_MSG_HEADER (sizeof(web_filter_msg_hdr))
/* netlink msg header */
typedef struct
{
    unsigned char  type;     // white or black list
    unsigned char  action;   // Add, del, purge, Get?
    unsigned short count;    // num of webfilter msg
}web_filter_msg_hdr;

#define LEN_WF_MSG_CONT    (sizeof(web_filter_msg_cont))
/* netlink msg content */
typedef struct
{
    unsigned char  target;                  // matched target : drop, reject, redirect..
    unsigned char  type;                    // md5(url) or string(host)
    unsigned short length;                  // length of data
    unsigned char data[0];                  // md5 sum(host+uri) or string
}web_filter_msg_cont;

/* linked in data field of web_filter_msg_cont
 * one web_filter_msg_cont contains more than one elem data */
#define LEN_WF_ELEM_DATA    (sizeof(web_filter_elem_data))
typedef struct
{
    unsigned short type;
    unsigned short len;
    unsigned char  value[0];
}web_filter_elem_data;

/* ---------------------------------> */

#define WEBFILTER_HASH_BITS 12
#define WEBFILTER_HASH_SIZE (1 << WEBFILTER_HASH_BITS)

// test this : a[15] is a char, not int
#define KEYGEN(a) ((uint)a[15] << 24 | (uint)a[11] << 16 | (uint)a[7] << 8 | (uint)a[3])
#define webfilter_md5_hashfn(uri) hash_long(KEYGEN(uri), WEBFILTER_HASH_BITS)
#define webfilter_str_hashfn(str) hash_long(BKDRHash(str), WEBFILTER_HASH_BITS)

// webfilter node
typedef struct
{
    union
    {
        struct hlist_node hash_node;
        struct list_head  list_head;
    };
    web_filter_msg_cont msg;
}web_filter_node;

/* operations of web filter, for different algorithm */
struct web_filter_data_operations
{
    void *  (*init_web_filter_table)(int size);
    void (*fini_web_filter_table)(void * data);
    void (*purge_web_filter_table)(void * data);
    void (*purge_special_data)(void * data, const web_filter_msg_cont * msg);
    void (*add_web_filter_node)(void * data, const web_filter_msg_cont * msg);
    void (*del_web_filter_node)(void * data, const web_filter_msg_cont * msg);
    //static web_filter_node * query_web_filter(unsigned char * md5_hash);
};

struct web_filter_data
{
    int type; // ops type, hash table or list
    int size; // hlist_head or list_head num
    atomic_t nums; // nums of rule
    union
    {
        struct hlist_head * htable;
        struct list_head  * list;
    };
    struct web_filter_data_operations * ops;
};

/* record http request info */
struct http_request_info
{
    char * url;
    char * host;
    char * refer;
//    spinlock_t lock;
};

/*==================== function declaration =====================*/
static void * webfilter_hash_init(int size);
static void webfilter_hash_fini(void * data);
static void webfilter_hash_add(void * data, const web_filter_msg_cont * msg);
static void webfilter_hash_del(void * data, const web_filter_msg_cont * msg);
static void webfilter_htable_purge(void * data);
static void webfilter_purge_special(void * data, const web_filter_msg_cont * msg);

static void * webfilter_string_list_init(int size);
static void webfilter_string_list_fini(void * data);
static void webfilter_string_list_add(void * data, const web_filter_msg_cont * msg);
static void webfilter_string_list_del(void * data, const web_filter_msg_cont * msg);
static void webfilter_string_list_purge(void * data);

static int alloc_web_filter_data(void);
static void free_web_filter_data(void);

static int init_web_filter_mmap(void);
static void fini_web_filter_mmap(void);

static int init_web_filter_sysctl(void);
static void fini_web_filter_sysctl(void);

static inline const char *findend(const char *data, const char *tail, int min);
static int md5_hash(const char *str, u32 len, u8 *hash);

/*=============== vars definition ==================*/
// netlink socket
struct sock* nl_sk = NULL;

// hash tables
//static struct hlist_head * url_htable = NULL;
//static struct list_head  * host_list  = NULL;

// pid of client
static u32 pid_webfilter_client = 0;

// network order "GET "
static const unsigned int constant_get_string = __constant_htonl(0x47455420);

// uri and host buf
#define LEN_WEBFILTER_URI   1024
#define LEN_WEBFILTER_URL   1024
#define LEN_WEBFILTER_HOST  1024
#define LEN_WEBFILTER_REFER 1024
#define MAX_DOMAIN_LEN      256
#define HTTP_PROT_SCHEMA    "http://"

// save http package info
static struct http_request_info * http_info = NULL;

// webfilter slab cache
struct kmem_cache* webfilter_url_cachep = NULL;
struct kmem_cache* webfilter_host_cachep = NULL;

/* spinlock for match function */
static DEFINE_SPINLOCK(webfilter_spin_lock);
/* mutex lock for netlink */
static DEFINE_MUTEX(webfilter_mutex_lock);

/* web filter operations type, corresponding with list type */
enum {
    URL_WEBFILTER_OPS = 0,
    HOST_WEBFILTER_OPS,
    MAX_WEBFILTER_OPS_CNT
};
/* web filter data type */
enum {
    URL_TABLE_TYPE = 0,
    HOST_TABLE_TYPE,
    STRING_LIST_TYPE
};

/* hash table operations*/
static struct web_filter_data_operations htable_ops = {
    .init_web_filter_table = webfilter_hash_init,
    .fini_web_filter_table = webfilter_hash_fini,
    .add_web_filter_node = webfilter_hash_add,
    .del_web_filter_node = webfilter_hash_del,
    .purge_web_filter_table = webfilter_htable_purge,
    .purge_special_data     = webfilter_purge_special,
};

/* string list operation */
static struct web_filter_data_operations list_string_ops = {
    .init_web_filter_table = webfilter_string_list_init,
    .fini_web_filter_table = webfilter_string_list_fini,
    .add_web_filter_node = webfilter_string_list_add,
    .del_web_filter_node = webfilter_string_list_del,
    .purge_web_filter_table = webfilter_string_list_purge,
    .purge_special_data     = NULL,
};

/* web filter operations point */
static struct web_filter_data * webfilter_data[MAX_WEBFILTER_OPS_CNT];

/* operation of url type */
static struct web_filter_data url_webfilter_data = {
    .type   = URL_TABLE_TYPE,
    .size   = WEBFILTER_HASH_SIZE,
    .nums   = ATOMIC_INIT(0),
    .htable = NULL,
    .ops    = &htable_ops
};

/* operation of host type : md5 */
static struct web_filter_data host_webfilter_data = {
    .type   = HOST_TABLE_TYPE,
    .size   = WEBFILTER_HASH_SIZE,
    .nums   = ATOMIC_INIT(0),
    .htable = NULL,
    .ops    = &htable_ops
};
/*
// string list
static struct web_filter_data host_webfilter_data = {
    .type = STRING_LIST_TYPE,
    .size = 1,
    .nums = ATOMIC_INIT(0),
    .list = NULL,
    .ops  = &list_string_ops
};
*/

/* sysctl of http match module */
#define LEN_WEBFILTER_NETWORK   2
#define HTTP_MATCH_DEF_IP   0xc0a81f00
#define HTTP_MATCH_DEF_MASK 0xffffff00
/* local redirect url */
#define HTTP_MATCH_DEF_SEC_WARN_URL  "http://192.168.31.1/sec_warnning.html"

static int zero = 0;
static int sysctl_http_switch_off = 0;
static int sysctl_http_switch_on  = 1;
static int sysctl_http_match_debug_switch __read_mostly;
static int sysctl_http_ad_filter_switch __read_mostly;
static unsigned int sysctl_http_match_network[LEN_WEBFILTER_NETWORK] __read_mostly;
static unsigned char sysctl_http_match_warn_page[MD5_HASH_LEN + 1] __read_mostly;
static int sysctl_http_matched_count __read_mostly;
struct ctl_table_header * sysctl_http_match_hdr = NULL;

static struct ctl_table sysctl_http_match_table[] = {
    {
        .procname   = "http_match_debug_switch",
        .data       = &sysctl_http_match_debug_switch,
        .maxlen     = sizeof(int),
        .mode       = 0644,
        .proc_handler   = proc_dointvec_minmax,
        .extra1         = &sysctl_http_switch_off,
        .extra2         = &sysctl_http_switch_on
    },
    {
        .procname   = "http_match_network",
        .data       = &sysctl_http_match_network,
        .maxlen     = sizeof(sysctl_http_match_network),
        .mode       = 0644,
        .proc_handler   = proc_dointvec
    },
    {
        .procname   = "http_match_warn_page",
        .data       = &sysctl_http_match_warn_page,
        .maxlen     = sizeof(sysctl_http_match_warn_page),
        .mode       = 0644,
        .proc_handler   = proc_dostring
    },
    {
        .procname   = "http_matched_count",
        .data       = &sysctl_http_matched_count,
        .maxlen     = sizeof(int),
        .mode       = 0644,
        .proc_handler   = proc_dointvec_minmax,
        .extra1         = &zero,
    },
    {
        .procname   = "http_ad_filter_switch",
        .data       = &sysctl_http_ad_filter_switch,
        .maxlen     = sizeof(int),
        .mode       = 0644,
        .proc_handler   = proc_dointvec_minmax,
        .extra1         = &sysctl_http_switch_off,
        .extra2         = &sysctl_http_switch_on
    },
    {}
};

/* for procfs of http match module */
#define HTTP_MATCH_MMAP_PAGE_ORDER  2
struct proc_dir_entry * proc_http_match_dir = NULL;
static unsigned long http_match_memaddr = 0;
static unsigned long http_match_memsize = 0;
#define __WF_MEM_DEBUG__
#ifdef __WF_MEM_DEBUG__
static atomic_t http_match_used_mem = ATOMIC_INIT(0);
static inline void update_mem_usage(int size, int is_add);
#endif

/* match statistic */
// sysctl_http_matched_count mean total matched count
static atomic_t url_match_cnt = ATOMIC_INIT(0);
static atomic_t host_match_cnt = ATOMIC_INIT(0);
static atomic_t sec_match_cnt = ATOMIC_INIT(0);
static atomic_t ad_match_cnt = ATOMIC_INIT(0);

/* for md5 hash crypto */
struct crypto_shash * crypto_shash_md5 = NULL;
struct shash_desc * sdescmd5 = NULL;

/*==================== function definition ====================*/

/* get_elem_data : get elem->value by LIST_TYPE
 * int list_type : list type
 * web_filter_msg_cont * msg : the msg want to access.
 * return value : None
 */
static inline const unsigned char * get_elem_data(int list_type, const web_filter_msg_cont * msg)
{
    if (URL_TABLE_TYPE == list_type)
    {
        return msg->data;
    }
    else if (HOST_TABLE_TYPE == list_type)
    {
        const web_filter_elem_data * elem = (const web_filter_elem_data *)msg->data;
        return elem->value;
    }
    else
    {
        printk(KERN_ERR"%s, fatal error!!!\n", __FUNCTION__);
        return NULL;
    }
}

/* get_elem_type : get elem->type
 * int list_type : list type
 * web_filter_msg_cont * msg : the msg want to access.
 * return value : type of elem
 */
static inline const int get_elem_type(int list_type, const web_filter_msg_cont * msg)
{
    if (HOST_TABLE_TYPE == list_type && NULL != msg)
    {
        const web_filter_elem_data * elem = (const web_filter_elem_data *)msg->data;
        return elem->type;
    }
    else
    {
        return -1;
    }
}

static inline web_filter_node * alloc_webfilter_node(int list_type, gfp_t flags)
{
    if (URL_TABLE_TYPE == list_type)
    {
        return kmem_cache_alloc(webfilter_url_cachep, flags);
    }
    else if (HOST_TABLE_TYPE == list_type)
    {
        return kmem_cache_alloc(webfilter_host_cachep, flags);
    }
    else
    {
        printk(KERN_ERR"%s, unknow list type!\n", __func__);
        return NULL;
    }
}

static inline void free_webfilter_node(int list_type, web_filter_node * node)
{
    if (URL_TABLE_TYPE == list_type)
    {
        kmem_cache_free(webfilter_url_cachep, node);
    }
    else if (HOST_TABLE_TYPE == list_type)
    {
        kmem_cache_free(webfilter_host_cachep, node);
    }
    else
    {
        printk(KERN_ERR"%s, unknow list type!\n", __func__);
    }
}

#ifdef __WF_DEBUG__
static inline void print_webfilter_msg_cont(web_filter_msg_cont * msg)
{
    LOG("=>msg target=%d, list=%d, len=%d\n\tdata=", msg->target, msg->type, msg->length);

    if (WEBFILTER_LIST_URL == msg->type)
    {
        LOGHEX(msg->data, msg->length);
    }
    else if (WEBFILTER_LIST_HOST == msg->type)
    {
        //LOG("%s\n", msg->data);
        web_filter_elem_data * elem = (web_filter_elem_data *)msg->data;
        LOG("(rule=0x%0x, len=%d):", elem->type, elem->len);
        LOGHEX(elem->value, elem->len);
    }
}
#else
static inline void print_webfilter_msg_cont(web_filter_msg_cont * msg) {}
#endif

#ifdef __WF_MEM_DEBUG__
static inline void update_mem_usage(int size, int is_add)
{
    if (is_add)
    {
        atomic_add(size, &http_match_used_mem);
    }
    else
    {
        atomic_sub(size, &http_match_used_mem);
    }
}
#else
static inline void update_mem_usage(int size, int is_add) {}
#endif

/* md5_init : init md5 calc
 * return value : OK : 0, error : errno -1
 */
static int md5_init(void)
{
    u32 size=0;
    int err = 0;

    crypto_shash_md5 = crypto_alloc_shash("md5", 0, 0);
    if (IS_ERR(crypto_shash_md5))
    {
        return -1;
    }
    size = sizeof(struct shash_desc) + crypto_shash_descsize(crypto_shash_md5);
    sdescmd5 = kmalloc(size, GFP_KERNEL);
    if (!sdescmd5)
    {
        err = -1;
        goto malloc_err;
    }
    sdescmd5->tfm = crypto_shash_md5;
    sdescmd5->flags = 0x0;
    return 0;

malloc_err:
      crypto_free_shash(crypto_shash_md5);
      return err;
}

/* md5_cleanup  : cleanup md5 shash
 * return value : None
 */
static void md5_cleanup(void)
{
    if (sdescmd5)
    {
        kfree(sdescmd5);
    }
    crypto_free_shash(crypto_shash_md5);
}

/* md5_hash : calc md5 sum of str, store in hash buffer
 * char * str : string point
 * u32    len : len of string
 * u8    hash : md5 sum string, size = 16
 * return value : OK : 0, error : errno -1
 */
static int md5_hash(const char *str, u32 len, u8 *hash)
{
    int err = 0;
    err = crypto_shash_init(sdescmd5);
    if (err) {
        err = -1;
        goto hash_err;
    }
    crypto_shash_update(sdescmd5, str, len);
    err = crypto_shash_final(sdescmd5, hash);

hash_err:
    return err;
}

/* BKDRHash   : BKDR Hash Function
 * char * str : string point
 * return value : hash value
 */
unsigned int BKDRHash(const unsigned char *str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str)
    {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

/**
 * strrstr - Back search the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 * modify from strstr in lib/string.c
 */
char *strrstr(const char *s1, const char *s2)
{
    size_t l1, l2;
    const char * p = NULL;
    int len = 0;

    l2 = strlen(s2);
    if (!l2)
        return (char *)s1;
    l1 = strlen(s1);
    len = l1 - l2;
    if (len >= 0)
    {
        p = s1 + len;
        while (p >= s1)
        {
            if (!memcmp(p, s2, l2))
                return (char *)p;
            p--;
        }
    }
    return NULL;
}

/***********************web_filter_ops -- hash table **************************/
/* webfilter_hash_add : add to hash table
 * void * head : hash table point
 * web_filter_msg_cont * msg : the msg want to add.
 * return value : None
 */
static void webfilter_hash_add(void * data, const web_filter_msg_cont * msg)
{
    struct web_filter_data * pdata = data;
    struct hlist_head * htable = NULL;
    web_filter_node * node = NULL;

    htable = pdata->htable;
    if (NULL == htable)
    {
        return;
    }

    mutex_lock(&webfilter_mutex_lock);
    //node = kzalloc(sizeof(web_filter_node) + msg->length, GFP_KERNEL);
    node = alloc_webfilter_node(pdata->type, GFP_KERNEL);
    if (NULL == node)
    {
        LOG("%s, kzalloc web_filter_node failed!\n", __FUNCTION__);
        mutex_unlock(&webfilter_mutex_lock);
        return;
    }

    memcpy((void*)&node->msg, msg, sizeof(web_filter_msg_cont) + msg->length);
    hlist_add_head_rcu(&node->hash_node, &htable[webfilter_md5_hashfn(get_elem_data(pdata->type, msg))]);
    // udpate rule nums
    atomic_inc(&pdata->nums);
    update_mem_usage(sizeof(web_filter_node) + msg->length, 1);
    LOG("%s, insert at %d, total rule nums=%d\n", __FUNCTION__, webfilter_md5_hashfn(get_elem_data(pdata->type, msg)), atomic_read(&pdata->nums));
    mutex_unlock(&webfilter_mutex_lock);
}

/* webfilter_hash_del : del from hash table
 * void * head : hash table point
 * web_filter_msg_cont * msg : the msg want to del.
 * return value : None
 */
static void webfilter_hash_del(void * data, const web_filter_msg_cont * msg)
{
    struct web_filter_data * pdata = data;
    struct hlist_head * htable = NULL;
    web_filter_node * node = NULL;
    struct hlist_node * hnode1, *tmp1;
    struct hlist_head * hash_head;

    htable = pdata->htable;
    if (NULL == htable)
    {
        return;
    }

    // find and del
    mutex_lock(&webfilter_mutex_lock);
    spin_lock_bh(&webfilter_spin_lock);

    hash_head = &htable[webfilter_md5_hashfn(get_elem_data(pdata->type, msg))];
    hlist_for_each_entry_safe(node, hnode1, tmp1, hash_head, hash_node)
    {
        if (0 == memcmp(get_elem_data(pdata->type, &node->msg), get_elem_data(pdata->type, msg), MD5_HASH_LEN))
        {
            hlist_del(&node->hash_node);
            free_webfilter_node(pdata->type, node);
            // update rule nums
            atomic_dec(&pdata->nums);
            update_mem_usage((sizeof(web_filter_node) + node->msg.length), 0);
            LOG("%s, del URL, find at %d, left rule nums=%d\n", __FUNCTION__,
                    webfilter_md5_hashfn(get_elem_data(pdata->type, msg)), atomic_read(&pdata->nums));
            break;
        }
    }
    spin_unlock_bh(&webfilter_spin_lock);
    mutex_unlock(&webfilter_mutex_lock);
}

/* webfilter_purge_special : purge special type data in table
 * void * data : hash table point
 * web_filter_msg_cont * msg : the msg contain special type.
 * return value : None
 */
static void webfilter_purge_special(void * data, const web_filter_msg_cont * msg)
{
    int i = 0;
    int cnt = 0;
    struct web_filter_data * pdata = data;
    web_filter_node  * node = NULL;
    struct hlist_node * hnode1, *tmp1;
    struct hlist_head * hash_head;
    struct hlist_head * htable = NULL;
    int etype = -1;

    htable = pdata->htable;
    if (NULL == htable)
    {
        return;
    }

    // get type you want to purge
    etype = get_elem_type(pdata->type, msg);
    if (etype < WEBFILTER_RULE_NONE || etype >= WEBFILTER_RULE_MAX)
    {
        printk(KERN_ERR"%s, error etype!!!\n", __FUNCTION__);
        return;
    }
    mutex_lock(&webfilter_mutex_lock);
    spin_lock_bh(&webfilter_spin_lock);
    // free all hlist
    for (i = 0; i < pdata->size; i++)
    {
        hash_head = &htable[i];
        if (hash_head->first)
        {
            hlist_for_each_entry_safe(node, hnode1, tmp1, hash_head, hash_node)
            {
                // free node
                //LOG("free node: ");
                //print_webfilter_msg_cont(&node->msg);
                if (etype & get_elem_type(pdata->type, &node->msg))
                {
                    hlist_del(&node->hash_node);
                    free_webfilter_node(pdata->type, node);
                    update_mem_usage((sizeof(web_filter_node) + node->msg.length), 0);
                    atomic_dec(&pdata->nums);
                    cnt++;
                }
            }
        }
    }
    spin_unlock_bh(&webfilter_spin_lock);
    mutex_unlock(&webfilter_mutex_lock);
    printk("%s, purge %d rules, total rule nums=%d\n", __FUNCTION__, cnt, atomic_read(&pdata->nums));
}

/*
 * remove and free all node from htable
 * 为节省空间没有保存rcu head，所以使用hlist_del_rcu/synchronize_rcu/free清空表的效率很低
 * 清空表的时候先删除iptable规则或卸载模块，然后就可以安全删除
 */
static void webfilter_htable_purge(void * data)
{
    int i = 0;
    int cnt = 0;
    struct web_filter_data * pdata = data;
    web_filter_node  * node = NULL;
    struct hlist_node * hnode1, *tmp1;
    struct hlist_head * hash_head;
    struct hlist_head * htable = NULL;

    htable = pdata->htable;
    if (NULL == htable)
    {
        return;
    }

    mutex_lock(&webfilter_mutex_lock);
    spin_lock_bh(&webfilter_spin_lock);
    // free all hlist
    for (i = 0; i < pdata->size; i++)
    {
        hash_head = &htable[i];
        if (hash_head->first)
        {
            hlist_for_each_entry_safe(node, hnode1, tmp1, hash_head, hash_node)
            {
                // free node
                //LOG("free node: ");
                //print_webfilter_msg_cont(&node->msg);
                hlist_del(&node->hash_node);
                free_webfilter_node(pdata->type, node);
                update_mem_usage((sizeof(web_filter_node) + node->msg.length), 0);
                cnt++;
            }
        }
    }
    // reset rule nums to zero
    atomic_set(&pdata->nums, 0);
    spin_unlock_bh(&webfilter_spin_lock);
    mutex_unlock(&webfilter_mutex_lock);
    printk("%s, purge %d rules, total rule nums=%d\n", __FUNCTION__, cnt, atomic_read(&pdata->nums));
}

/* webfilter_hash_init  : init hash table
 * web_filter_ops * ops : webfilter operations
 * int size             : size of hash table
 * return value : 0:success, negtive value if failed.
 */
static void * webfilter_hash_init(int size)
{
    int i = 0;
    struct hlist_head * htable = NULL;

    htable = (struct hlist_head *)kmalloc(size * sizeof(struct hlist_head), GFP_KERNEL);
    if (NULL == htable)
    {
        printk(KERN_ERR "%s, kmalloc(size:%d) hash table failed, return!\n", __FUNCTION__, size);
        return NULL;
    }
    for (i = 0; i < size; i++)
    {
        INIT_HLIST_HEAD(&(htable[i]));
    }

    LOG("%s, succeed!\n", __FUNCTION__);
    return htable;
}

/* webfilter_hash_fini  : destory hash table
 * web_filter_ops * ops : webfilter operations
 * return value : None
 */
static void webfilter_hash_fini(void * data)
{
    struct web_filter_data * pdata = data;

    if (pdata && pdata->htable)
    {
        // purge table
        webfilter_htable_purge(pdata);
        // free htable
        kfree(pdata->htable);
        pdata->htable = NULL;
    }
}

/***********************web_filter_ops -- string list **************************/
/* webfilter_string_list_init : init host list
 * web_filter_ops * ops: web filter operation
 * int    size : list size, here ignore it.
 * return value : 0:success, negtive value if failed
 */
static void * webfilter_string_list_init(int size)
{
    struct list_head * list = NULL;

    list = (struct list_head *)kmalloc(sizeof(struct list_head), GFP_KERNEL);
    if (NULL == list)
    {
        printk(KERN_ERR "%s, kmalloc(size:%d) list failed, return!\n", __FUNCTION__, size);
        return NULL;
    }
    INIT_LIST_HEAD(list);
    LOG("%s, succeed!\n", __FUNCTION__);
    return list;
}

/* webfilter_string_list_fini : destroy string list
 * web_filter_ops * ops: web filter operation
 * return value : None
 */
static void webfilter_string_list_fini(void * data)
{
    struct web_filter_data * pdata = data;

    if (pdata && pdata->list)
    {
        webfilter_string_list_purge(pdata);
        kfree(pdata->list);
        pdata->list = NULL;
    }
}

/* webfilter_string_list_purge : clean all data in string list
 * void * head : string list head
 * return value : None
 */
static void webfilter_string_list_purge(void * data)
{
    int cnt = 0;
    struct web_filter_data * pdata = data;
    struct list_head * list = NULL;
    web_filter_node * node = NULL;
    web_filter_node * tmp  = NULL;

    list = pdata->list;
    if (NULL == list)
    {
        return;
    }

    mutex_lock(&webfilter_mutex_lock);
    spin_lock_bh(&webfilter_spin_lock);
    list_for_each_entry_safe(node, tmp, list, list_head)
    {
        //LOG("free URL node : ");
        //print_webfilter_msg_cont(&node->msg);
        list_del(&node->list_head);
        kfree(node);
        cnt++;
    }
    // reset rule nums to zero
    atomic_set(&pdata->nums, 0);
    spin_unlock_bh(&webfilter_spin_lock);
    mutex_unlock(&webfilter_mutex_lock);
    printk("%s, purge %d rules, %d rules left.\n", __FUNCTION__, cnt, atomic_read(&pdata->nums));
}

/* webfilter_string_list_add : add msg to list
 * void * head : list head point
 * web_filter_msg_cont * msg : the msg want to add
 * return value : None
 */
static void webfilter_string_list_add(void * data, const web_filter_msg_cont * msg)
{
    struct web_filter_data * pdata = data;
    struct list_head * list_head = NULL;
    web_filter_node * node = NULL;

    list_head = pdata->list;
    if (NULL == list_head)
    {
        return;
    }

    mutex_lock(&webfilter_mutex_lock);
    node = kzalloc(sizeof(web_filter_node) + msg->length, GFP_KERNEL);
    if (NULL == node)
    {
        LOG("%s, kzalloc web_filter_node failed!\n", __FUNCTION__);
        mutex_unlock(&webfilter_mutex_lock);
        return;
    }

    memcpy((void*)&node->msg, msg, sizeof(web_filter_msg_cont) + msg->length);
    list_add_tail_rcu(&node->list_head, list_head);
    // update rule nums
    atomic_inc(&pdata->nums);
    mutex_unlock(&webfilter_mutex_lock);
}

/* webfilter_string_list_del : del from list
 * void * head : list head point
 * web_filter_msg_cont * msg : the msg want to del
 * return value : None
 */
static void webfilter_string_list_del(void * data, const web_filter_msg_cont * msg)
{
    struct web_filter_data * pdata = data;
    struct list_head * head = NULL;
    web_filter_node * node = NULL;
    web_filter_node * tmp  = NULL;

    head = pdata->list;
    if (NULL == head)
    {
        return;
    }

    // find and del
    mutex_lock(&webfilter_mutex_lock);
    spin_lock_bh(&webfilter_spin_lock);
    list_for_each_entry_safe(node, tmp, head, list_head)
    {
        if (0 == memcmp(msg->data, node->msg.data, msg->length))
        {
            //LOG("free Host node: ");
            //print_webfilter_msg_cont(&node->msg);
            list_del(&node->list_head);
            kfree(node);
            // udpate rule nums
            atomic_dec(&pdata->nums);
            break;
        }
    }
    spin_unlock_bh(&webfilter_spin_lock);
    mutex_unlock(&webfilter_mutex_lock);
}

/*********************** http head info **************************/

static void free_http_info(void)
{
    if (http_info)
    {
        if (http_info->url)
        {
            kfree(http_info->url);
        }
        if (http_info->host)
        {
            kfree(http_info->host);
        }
        if (http_info->refer)
        {
            kfree(http_info->refer);
        }
        kfree(http_info);
        http_info = NULL;
    }
}

static int alloc_http_info(void)
{
    memset((void*)&http_info, 0, sizeof(struct http_request_info));
    http_info = kzalloc(sizeof(struct http_request_info), GFP_KERNEL);
    if (NULL == http_info)
    {
        printk("%s, alloc http info failed!\n", __FUNCTION__);
        return -1;
    }

    http_info->url   = kzalloc(LEN_WEBFILTER_URL,    GFP_KERNEL);
    http_info->host  = kzalloc(LEN_WEBFILTER_HOST,   GFP_KERNEL);
    http_info->refer = kzalloc(LEN_WEBFILTER_REFER,  GFP_KERNEL);

    if (NULL == http_info->url || NULL == http_info->host || NULL == http_info->refer)
    {
        printk("%s, alloc http info member failed!\n", __FUNCTION__);
        free_http_info();
        return -1;
    }
    //spin_lock_init(&http_info->lock);
    return 0;
}

/*********************** sysctl of web filter **************************/

static int init_web_filter_sysctl(void)
{
    memset((void*)&sysctl_http_match_debug_switch, 0, sizeof(sysctl_http_match_debug_switch));
    memset((void*)&sysctl_http_match_network, 0, sizeof(sysctl_http_match_network));
    memset((void*)&sysctl_http_match_warn_page, 0, sizeof(sysctl_http_match_warn_page));

    // set default value
    sysctl_http_match_debug_switch = sysctl_http_switch_off;
    sysctl_http_ad_filter_switch = sysctl_http_switch_off;
    sysctl_http_match_network[0] = __constant_htonl(HTTP_MATCH_DEF_IP);
    sysctl_http_match_network[1] = __constant_htonl(HTTP_MATCH_DEF_MASK);
    sysctl_http_matched_count = 0;

    //md5 sum of default warnning page
    md5_hash(HTTP_MATCH_DEF_SEC_WARN_URL, strlen(HTTP_MATCH_DEF_SEC_WARN_URL), sysctl_http_match_warn_page);
    LOG("warn page md5 sum:");
    LOGHEX(sysctl_http_match_warn_page, MD5_HASH_LEN);

    // init sysfs
    sysctl_http_match_hdr = register_sysctl_paths(net_ipv4_ctl_path, sysctl_http_match_table);
    if (NULL == sysctl_http_match_hdr)
    {
        printk("%s, failed!\n", __FUNCTION__);
        return -ENOMEM;
    }
    return 0;
}

static void fini_web_filter_sysctl(void)
{
    if (sysctl_http_match_hdr)
    {
        unregister_sysctl_table(sysctl_http_match_hdr);
    }
}

static inline void inc_counter(atomic_t * counter)
{
    if (atomic_read(counter) == INT_MAX)
    {
        atomic_set(counter, 0);
    }
    atomic_inc(counter);
}

static void update_webfilter_match_count(int list_type, int rule_type)
{
    // update all-type's counter
    if (sysctl_http_matched_count == INT_MAX)
    {
        sysctl_http_matched_count = 0;
    }
    else
    {
        ++sysctl_http_matched_count;
    }

    // update by type
    if (WEBFILTER_LIST_HOST == list_type)
    {
        inc_counter(&host_match_cnt);
        if (WEBFILTER_RULE_SECURITY & rule_type)
        {
            inc_counter(&sec_match_cnt);
        }
        else if (WEBFILTER_RULE_AD & rule_type)
        {
            inc_counter(&ad_match_cnt);
        }
        else
        {
            // impossible
        }
    }
    else
    {
        // default as url
        inc_counter(&url_match_cnt);
    }
}

static inline int is_ad_filter_enabled(void)
{
    return sysctl_http_ad_filter_switch;
}

/*********************** mmap interface of web filter **************************/

static int get_mmap_pages()
{
    http_match_memaddr = __get_free_pages(GFP_KERNEL | __GFP_ZERO, HTTP_MATCH_MMAP_PAGE_ORDER);

    if (http_match_memaddr)
    {
        int i = 0;
        int count = 1 << HTTP_MATCH_MMAP_PAGE_ORDER;
        struct page * pages = virt_to_page(http_match_memaddr);
        for (i = 0; i < count; i++)
        {
            SetPageReserved(pages + i);
        }
        http_match_memsize = PAGE_SIZE * count;
    }
    else
    {
        LOG("%s, get free page failed, return!\n", __FUNCTION__);
        return -ENOMEM;
    }
    return 0;
}

static void free_mmap_pages()
{
    // free pages
    if (http_match_memaddr)
    {
        unsigned int count = 1 << HTTP_MATCH_MMAP_PAGE_ORDER;
        unsigned int i = 0;
        struct page * pages = virt_to_page(http_match_memaddr);

        for (i = 0; i < count; i++)
        {
            ClearPageReserved(pages + i);
        }
        __free_pages(pages, HTTP_MATCH_MMAP_PAGE_ORDER);
        http_match_memaddr = 0;
        http_match_memsize = 0;
    }
}

static int mtp_mmap(struct file *filp, struct vm_area_struct *vma)
{
    size_t size = vma->vm_end - vma->vm_start;
    unsigned long phys = virt_to_phys(http_match_memaddr);

    if (remap_pfn_range(vma, vma->vm_start, phys >> PAGE_SHIFT, size, vma->vm_page_prot))
    {
        printk("%s, remap_pfn_range failed!\n", __func__);
        return -1;
    }

    printk("%s, process(%d) mmap, size=%d!\n", __FUNCTION__, current->pid, size);
    return 0;
}

static int mtp_close(struct inode *inode, struct file *filp)
{
    printk("%s, process(%d) closed!\n", __FUNCTION__, current->pid);
    return 0;
}

static int mtp_open(struct inode *inode, struct file *filp)
{
    // if mmap alloc failed before, retry
    if (0 == http_match_memaddr)
    {
        if (get_mmap_pages())
        {
            return -ENOMEM;
        }
    }
    printk("%s, process(%d) opened!\n", __FUNCTION__, current->tgid);
    return 0;
}

#define MMAP_DEVNAME "mmap_tcp_proxy"

static const struct file_operations mmap_fops = {
    .open = mtp_open,
    .release = mtp_close,
    .mmap = mtp_mmap,
};

static struct miscdevice mmap_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = MMAP_DEVNAME,
    .fops = &mmap_fops,
};

static int proc_read_webfilter_mem_addr(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
    *eof = 1;
    return sprintf(page, "%08lx\n", __pa(http_match_memaddr));
}

static int proc_read_webfilter_mem_size(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
    *eof = 1;
    return sprintf(page, "%lu\n", http_match_memsize);
}

#ifdef __WF_MEM_DEBUG__
static int proc_read_webfilter_used_mem(char * page, char ** start, off_t off,
        int count, int *eof, void *data)
{
    *eof = 1;
    return sprintf(page, "%d\n", atomic_read(&http_match_used_mem));
}

static int proc_read_webfilter_rule_nums(char * page, char ** start, off_t off,
        int count, int *eof, void *data)
{
    *eof = 1;
    return sprintf(page, "url:%d; host:%d\n",
                atomic_read(&webfilter_data[URL_TABLE_TYPE]->nums),
                atomic_read(&webfilter_data[HOST_TABLE_TYPE]->nums));
}
#endif

static int proc_read_webfilter_match_stats(char * page, char ** start, off_t off,
        int count, int *eof, void *data)
{
    *eof = 1;
    return sprintf(page, "all:%d, url:%d; host:%d; sec:%d, ad:%d\n", sysctl_http_matched_count,
                atomic_read(&url_match_cnt), atomic_read(&host_match_cnt),
                atomic_read(&sec_match_cnt), atomic_read(&ad_match_cnt));
}

static int init_web_filter_mmap(void)
{
    if (get_mmap_pages())
    {
        return -1;
    }
    if (misc_register(&mmap_misc))
    {
        printk("%s, register misc dev failed!\n", __func__);
        return -1;
    }

    proc_http_match_dir = create_proc_entry("http_match", S_IFDIR, NULL);
    if (NULL == proc_http_match_dir)
    {
        LOG("%s, create http_match proc entry failed!\n", __FUNCTION__);
        return -1;
    }
    create_proc_read_entry("mem_addr", S_IRUGO, proc_http_match_dir, proc_read_webfilter_mem_addr, NULL);
    create_proc_read_entry("mem_size", S_IRUGO, proc_http_match_dir, proc_read_webfilter_mem_size, NULL);
#ifdef __WF_MEM_DEBUG__
    create_proc_read_entry("mem_used", S_IRUGO, proc_http_match_dir, proc_read_webfilter_used_mem, NULL);
    create_proc_read_entry("rule_nums", S_IRUGO, proc_http_match_dir, proc_read_webfilter_rule_nums, NULL);
#endif
    create_proc_read_entry("match_stats", S_IRUGO, proc_http_match_dir, proc_read_webfilter_match_stats, NULL);

    // alloc pages at mmap open, just return
    return 0;
}

static void fini_web_filter_mmap(void)
{
    remove_proc_entry("mem_addr", proc_http_match_dir);
    remove_proc_entry("mem_size", proc_http_match_dir);
#ifdef __WF_MEM_DEBUG__
    remove_proc_entry("mem_used", proc_http_match_dir);
    remove_proc_entry("rule_nums", proc_http_match_dir);
#endif
    remove_proc_entry("match_stats", proc_http_match_dir);
    remove_proc_entry("http_match", NULL);
    misc_deregister(&mmap_misc);
    free_mmap_pages();
}

/*********************** data init/destroy of web filter **************************/

static int create_webfilter_rule_cache()
{
    // NOTE: sizeof [slab cache elem] should be change if struct node changed.
    webfilter_url_cachep = kmem_cache_create("wf_url_cache",
                            sizeof(web_filter_node) + MD5_HASH_LEN, 0,
                            SLAB_HWCACHE_ALIGN | SLAB_PANIC, NULL);
    if (!webfilter_url_cachep)
    {
        printk(KERN_ERR "%s,  Failed to create webfilter url slab cache!\n");
        return -1;
    }
    webfilter_host_cachep = kmem_cache_create("wf_host_cache",
                            sizeof(web_filter_node) + MD5_HASH_LEN + sizeof(web_filter_elem_data), 0,
                            SLAB_HWCACHE_ALIGN | SLAB_PANIC, NULL);
    if (!webfilter_host_cachep)
    {
        printk(KERN_ERR "%s,  Failed to create webfilter host slab cache!\n");
        kmem_cache_destroy(webfilter_url_cachep);
        return -1;
    }
    LOG("%s, creat url and host slab cache succeed!\n", __func__);
    return 0;
}

static void destroy_webfilter_rule_cache()
{
    if (webfilter_url_cachep)
    {
        kmem_cache_destroy(webfilter_url_cachep);
    }
    if (webfilter_host_cachep)
    {
        kmem_cache_destroy(webfilter_host_cachep);
    }
}

static int alloc_web_filter_data(void)
{
    int i = 0;
    int err = 0;
    struct web_filter_data * pdata = NULL;

    if (alloc_http_info())
    {
        return -1;
    }

    if (create_webfilter_rule_cache())
    {
        free_http_info();
        return -1;
    }

    /* set webfilter_data array */
    webfilter_data[URL_WEBFILTER_OPS] = &url_webfilter_data;
    webfilter_data[HOST_WEBFILTER_OPS] = &host_webfilter_data;

    for (i = 0; i < sizeof(webfilter_data)/sizeof(struct web_filter_data *); i++)
    {
        pdata = webfilter_data[i];
        if (pdata->ops && pdata->ops->init_web_filter_table)
        {
            if (URL_TABLE_TYPE == pdata->type || HOST_TABLE_TYPE == pdata->type)
            {
                pdata->htable = pdata->ops->init_web_filter_table(pdata->size);
                if (NULL == pdata->htable)
                {
                    err = -1;
                    printk("%s, alloc hash table failed.\n", __FUNCTION__);
                    break;
                }
            }
            else if (STRING_LIST_TYPE == pdata->type)
            {
                pdata->list = pdata->ops->init_web_filter_table(pdata->size);
                if (NULL == pdata->list)
                {
                    err = -1;
                    printk("%s, alloc string list failed.\n", __FUNCTION__);
                    break;
                }
            }
        }
    }
    if (err)
    {
        free_web_filter_data();
        return -1;
    }
    return 0;
}

static void free_web_filter_data(void)
{
    int i = 0;
    struct web_filter_data * pdata = NULL;

    for (i = 0; i < sizeof(webfilter_data)/sizeof(struct web_filter_data *); i++)
    {
        pdata = webfilter_data[i];
        if (pdata->ops && pdata->ops->fini_web_filter_table)
        {
            pdata->ops->fini_web_filter_table(pdata);
        }
    }
    destroy_webfilter_rule_cache();
    free_http_info();
}

/*********************** query functions of web filter **************************/

static web_filter_node * webfilter_query_htable(int list_type, unsigned char * md5_hash)
{
    struct web_filter_data * pdata = NULL;
    struct hlist_head * hlist = NULL;

    pdata = webfilter_data[list_type];
    if (NULL == pdata || NULL == pdata->htable || 0 == atomic_read(&pdata->nums))
    {
        return NULL;
    }

    rcu_read_lock();
    hlist = pdata->htable;
    if (hlist)
    {
        web_filter_node * node = NULL;
        struct hlist_node * tmp1;
        struct hlist_head * hash_head;
        hash_head = &hlist[webfilter_md5_hashfn(md5_hash)];
        hlist_for_each_entry_rcu(node, tmp1, hash_head, hash_node)
        {
            if (0 == memcmp(get_elem_data(pdata->type, &node->msg), md5_hash, MD5_HASH_LEN))
            {
                LOG("%s, HASH matched, target=%d!\n", __FUNCTION__, node->msg.target);
                rcu_read_unlock();
                return node;
            }
        }
    }
    rcu_read_unlock();
    return NULL;
}

static web_filter_node * webfilter_query_host_list(int list_type, const char * host)
{
    struct web_filter_data * pdata = NULL;
    struct list_head * head = NULL;

    pdata = webfilter_data[list_type];
    if (NULL == pdata || NULL == pdata->list || 0 == atomic_read(&pdata->nums))
    {
        return NULL;
    }

    rcu_read_lock();
    head = pdata->list;
    if (head)
    {
        web_filter_node * node = NULL;
        list_for_each_entry_rcu(node, head, list_head)
        {
            //LOG("%s, compare with %s\n", __FUNCTION__, node->msg.data);
            if (strrstr(host, node->msg.data))
            {
                LOG("%s, Host matched, target=%d!\n", __FUNCTION__, node->msg.target);
                rcu_read_unlock();
                return node;
            }
        }
    }
    rcu_read_unlock();
    return NULL;
}

static web_filter_node * webfilter_query_host_split_search(int list_type, const char * host)
{
    const char * p = NULL;
    const char * end = NULL;
    unsigned char md5_sum[MD5_HASH_LEN] = {0};
    web_filter_node * node = NULL;
    struct web_filter_data * pdata = NULL;

    // split host with '.'
    if (NULL == host)
    {
        return NULL;
    }
    pdata = webfilter_data[list_type];
    if (NULL == pdata || NULL == pdata->htable || 0 == atomic_read(&pdata->nums))
    {
        return NULL;
    }

    // skip the last dot
    p = host;
    end = host + strlen(host) - 1;
    while (end > p && *end != '.')
    {
        end--;
    }
    if (end == p)
    {
        return NULL;
    }

    // hsearch md5
    md5_hash(p, strlen(p), md5_sum);
    LOG("host : %s, md5 hash : ", p);
    LOGHEX(md5_sum, MD5_HASH_LEN);
    node = webfilter_query_htable(list_type, md5_sum);
    if (node)
    {
        return node;
    }

    while(p < end)
    {
        if (*p == '.')
        {
            ++p;
            memset(md5_sum, 0, sizeof(md5_sum));
            md5_hash(p, strlen(p), md5_sum);
            LOG("host : %s, md5 hash : ", p);
            LOGHEX(md5_sum, MD5_HASH_LEN);
            node = webfilter_query_htable(list_type, md5_sum);
            if (node)
            {
                return node;
            }
            else
            {
                p++;
            }
        }
        else
        {
            p++;
        }
    }
    return NULL;
}

/* query_third_party_split : split query, if the host is refer from third-party
 * const char * host       : host field extract from refer
 * const char * md5_value  : matched md5 value
 * return value  : if not matched, it is refer from third-party, return "1", or else "0".
 */
static int query_third_party_split(const char * host, const char * md5_value)
{
    const char * p = NULL;
    const char * end = NULL;
    unsigned char md5_sum[MD5_HASH_LEN] = {0};

    if (NULL == host || NULL == md5_value)
    {
        return 0;
    }

    // skip the last dot
    p = host;
    end = host + strlen(host) - 1;
    while (end > p && *end != '.')
    {
        end--;
    }
    if (end == p)
    {
        return 0;
    }

    md5_hash(p, strlen(p), md5_sum);
    LOG("host : %s, md5 hash : ", p);
    LOGHEX(md5_sum, MD5_HASH_LEN);
    if (0 == memcmp(md5_value, md5_sum, MD5_HASH_LEN))
    {
        return 0;
    }
    while(p < end)
    {
        if (*p == '.')
        {
            ++p;
            memset(md5_sum, 0, sizeof(md5_sum));
            md5_hash(p, strlen(p), md5_sum);
            LOG("host : %s, md5 hash : ", p);
            LOGHEX(md5_sum, MD5_HASH_LEN);
            if (0 == memcmp(md5_value, md5_sum, MD5_HASH_LEN))
            {
                return 0;
            }
            else
            {
                p++;
            }
        }
        else
        {
            p++;
        }
    }
    LOG("host : %s, third-party matched!\n", host);
    return 1;
}

static int is_webfilter_data_empty(void)
{
    return 0 == atomic_read(&webfilter_data[URL_WEBFILTER_OPS]->nums)
        && 0 == atomic_read(&webfilter_data[HOST_WEBFILTER_OPS]->nums);
}

/*********************** netlink interface of web filter **************************/

static void hash_oper(int which, int action, const web_filter_msg_cont * msg)
{
    struct web_filter_data * pdata = NULL;

    if (which > WEBFILTER_LIST_HOST && which < WEBFILTER_LIST_URL)
    {
        printk("%s, invalid list type(%d)!\n", __FUNCTION__, which);
        return;
    }
    if (WEBFILTER_TYPE_ADD > action || WEBFILTER_TYPE_PURGE_SPECIAL < action)
    {
        printk("%s, invalid action type(%d)!\n", __FUNCTION__, action);
        return;
    }
    if ((WEBFILTER_TYPE_ADD == action || WEBFILTER_TYPE_DEL == action) && NULL == msg)
    {
        printk("%s, mismatch action type(%d), msg is NULL!\n", __FUNCTION__, action);
        return;
    }

    pdata = webfilter_data[which];
    if (NULL == pdata && NULL == pdata->ops)
    {
        printk("%s, not supported operations for this list type(%d)!\n", __FUNCTION__, which);
        return;
    }

    switch (action)
    {
        case WEBFILTER_TYPE_ADD:
            if (pdata->ops->add_web_filter_node)
            {
                pdata->ops->add_web_filter_node(pdata, msg);
            }
            break;
        case WEBFILTER_TYPE_DEL:
            if (pdata->ops->del_web_filter_node)
            {
                pdata->ops->del_web_filter_node(pdata, msg);
            }
            break;
        case WEBFILTER_TYPE_PURGE:
            if (pdata->ops->purge_web_filter_table)
            {
                pdata->ops->purge_web_filter_table(pdata);
            }
            else
            {
                printk("%s, table[%d] not support purge function!\n", __FUNCTION__, which);
            }
            break;
        case WEBFILTER_TYPE_PURGE_SPECIAL:
            if (pdata->ops->purge_special_data)
            {
                pdata->ops->purge_special_data(pdata, msg);
            }
            else
            {
                printk("%s, table[%d] not support special purge function!\n", __FUNCTION__, which);
            }
            break;
        default:
            break;
    }
}

// must support multi-rules parse....
static void nl_data_ready(struct sk_buff* __skb)
{
    struct sk_buff* skb;
    struct nlmsghdr* nlh;
    int nMsg = 0;
    int nBytes = 0;

    skb = skb_get(__skb);

    if (skb->len >= NLMSG_SPACE(0))
    {
        unsigned char * pmsg = NULL;
        unsigned char * pmem = NULL;
        web_filter_msg_hdr  * msg_hdr  = NULL;
        web_filter_msg_cont * msg_cont = NULL;

        nlh = nlmsg_hdr(skb);
        pid_webfilter_client = nlh->nlmsg_pid;
        LOG("===client thread [%d], send msg len=%d, skb len=%d\n",
                pid_webfilter_client, nlh->nlmsg_len, skb->len);

        if (skb->len < nlh->nlmsg_len)
        {
            LOG("===skb->len(%d) < nlmsg_len(%d), return!\n", skb->len, nlh->nlmsg_len);
            goto out;
        }

        pmsg = (unsigned char*)NLMSG_DATA(nlh);
        msg_hdr = (web_filter_msg_hdr*)pmsg;
        nMsg = msg_hdr->count;
        LOG("msg_header(type:%d, action:%d, count:%d)\n", msg_hdr->type, msg_hdr->action, msg_hdr->count);

        switch (nlh->nlmsg_type)
        {
            case NETLINK_MSG_FOLLOW:
                nBytes = nlh->nlmsg_len - NLMSG_HDRLEN;
                nBytes -= LEN_WF_MSG_HEADER;
                if (0 == nMsg)
                {
                    hash_oper(msg_hdr->type, msg_hdr->action, NULL);
                    break;
                }
                while (nMsg-- && nBytes > 0)
                {
                    msg_cont = (web_filter_msg_cont*)((unsigned char*)msg_hdr + LEN_WF_MSG_HEADER);
                    print_webfilter_msg_cont(msg_cont);
                    hash_oper(msg_hdr->type, msg_hdr->action, msg_cont);

                    // next msg
                    nBytes = nBytes - LEN_WF_MSG_CONT - msg_cont->length;
                    LOG("---left bytes : %d---\n", nBytes);
                }
                break;
            case NETLINK_MSG_MMAP:
                // get msg from share mem
                if (0 == http_match_memaddr)
                {
                    printk("%s, share memory maybe not initiliazed!\n", __FUNCTION__);
                    break;
                }

                pmem = (unsigned char *)http_match_memaddr;
                msg_cont = (web_filter_msg_cont*)pmem;
                while (nMsg-- && msg_cont->length > 0)
                {
                    print_webfilter_msg_cont(msg_cont);
                    hash_oper(msg_hdr->type, msg_hdr->action, msg_cont);
                    // next msg
                    LOG("---left msg num : %d---\n", nMsg);
                    pmem += LEN_WF_MSG_CONT + msg_cont->length;
                    msg_cont = (web_filter_msg_cont*)pmem;
                }
                break;
            default:
                break;
        }
    }
out:
    // never forgot "free"
    kfree_skb(skb);
    return;
}

static int init_netlink(void)
{
    // netlink socket for user input
    nl_sk = netlink_kernel_create(&init_net, NETLINK_WEB_FILTER, 0, nl_data_ready,
        NULL, THIS_MODULE);
    if (!nl_sk)
    {
        printk(KERN_ERR "%s, Cannot create netlink socket.\n", __FUNCTION__);
        return -EIO;
    }

    printk("%s, create socket ok.\n", __FUNCTION__);
    return 0;
}

static void clean_netlink(void)
{
    if (nl_sk)
    {
        netlink_kernel_release(nl_sk);
    }
}

/*==================== webfilter match ======================*/
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

/* get_strip_md5 : trim space of string, and calc md5 sum
 * char * data   : data begin point
 * char * tail   : data tail point
 * char * hash   : hash string
 * return value  : hash string
 */
static int get_strip_md5(const char *data, const char *tail, unsigned char * hash)
{
    int dlen;

    while ((data < tail) && (*data == ' ')) ++data;
    while ((tail > data) && (*(tail - 1) == ' ')) --tail;

    dlen = tail - data;

    md5_hash(data, dlen, hash);
    LOG("query data:");
    LOGSTR(data, dlen);
    LOGHEX(hash, MD5_HASH_LEN);

    return 0;
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

    if (*len >= LEN_WEBFILTER_URI)
    {
        LOG("===Web filter URI space NOT enough for this package!\n");
        return -1;
    }

    LOG("len=%d, query uri : ", *len);
    LOGSTR(begin, *len);
    return 0;
}

/* get_strip_url : trim space of string, combine url with host and uri 
 * char * data   : data begin point
 * char * tail   : data tail point
 * char * uri    : uri point
 * char * url    : url store point
 * return value  : url
 */
static int get_strip_url(const char *data, const char *tail, unsigned char * uri, unsigned char * url)
{
    int dlen = 0;
    int uri_len = 0;

    while ((data < tail) && (*data == ' ')) ++data;
    while ((tail > data) && (*(tail - 1) == ' ')) --tail;

    dlen = tail - data;
    if (dlen >= LEN_WEBFILTER_URL)
    {
        LOG("===Web filter Host space NOT enough for this packet!\n");
        return -1;
    }
    memcpy(url, data, dlen);
    url[dlen] = '\0';

    uri_len = strlen(uri);
    if (uri_len >= (LEN_WEBFILTER_URL - dlen))
    {
        LOG("===Web filter URL space NOT enough for this packet!\n");
        return -1;
    }

    memcpy(url+dlen, uri, uri_len);
    url[dlen+uri_len] = '\0';
    LOG("len=%d,%d, query url : %s\n", dlen, uri_len, url);

    return 0;
}

/* is_third_party_ad : determine is third-party ads according to refer and host
 * return value : return yes(1) or no(0)
 */
static int is_third_party_ad(web_filter_elem_data * elem, struct http_request_info * info, char flag)
{
    if (flag & WEBFILTER_GET_REFER_OK)
    {
        char * begin = NULL;
        char * end = NULL;
        char host[MAX_DOMAIN_LEN] = {0};
        // get host field from refer
        if (0 == strncmp(info->refer, HTTP_PROT_SCHEMA, strlen(HTTP_PROT_SCHEMA)))
        {
            memset((void*)host, 0, sizeof(host));
            begin = info->refer + strlen(HTTP_PROT_SCHEMA);
            end = strchr(begin, '/');
            if (NULL == end)
            {
                if (strlen(begin) < MAX_DOMAIN_LEN)
                {
                    strcpy(host, begin);
                }
            }
            else
            {
                if (end - begin < MAX_DOMAIN_LEN)
                {
                    strncpy(host, begin, end - begin);
                }
            }
            if (strlen(host))
            {
                // search if matched node, split refer and compare, return true if not match
                return query_third_party_split(host, elem->value);
            }
        }
    }
    return 0;
}

/* check_ad_filter : check ad filter switch, and is third-party links
 * return value : return target
 */
static int check_ad_filter(web_filter_msg_cont * msg, struct http_request_info * info, char flag)
{
    web_filter_elem_data * elem = (web_filter_elem_data *)msg->data;

    // if switch off, return 0 directly
    if (is_ad_filter_enabled())
    {
        if (WEBFILTER_RULE_AD_THIRD == elem->type)
        {
            if (is_third_party_ad(elem, info, flag))
            {
                return msg->target;
            }
        }
        else
        {
            return msg->target;
        }
    }
    return 0;
}

/* webfilter_mt : webfilter match function, NOTE:skb must be a valid GET packet
 * return value : return target
 */
int web_filter_match(const struct sk_buff *skb)
{
    const struct iphdr *iph = ip_hdr(skb);
    const struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
    const char *data = NULL;
    const char *tail = NULL;
    const char *p = NULL;
    int doff = 0;
    int dlen = 0;
    web_filter_node * node = NULL;
    unsigned char hash[MD5_HASH_LEN] = {0};
    const char * uri_data = NULL; // uri point in request
    int uri_len = 0;              // uri length
    char * url = NULL;            // url buffer
    int host_len = 0;             // host length
    char * host = NULL;           // host buffer
    char * refer = NULL;          // refer buffer
    int refer_len = 0;            // refer length
    char flag = 0;
    int target = 0;

    // src addr isn't LAN, skip
    if (sysctl_http_match_network[0] != (iph->saddr & sysctl_http_match_network[1]))
    {
        LOG("%s, src addr isn't LAN, skip\n", __FUNCTION__);
        return 0;
    }
    // dst addr is LAN, skip
    if (sysctl_http_match_network[0] == (iph->daddr & sysctl_http_match_network[1]))
    {
        LOG("%s, dst addr is LAN, skip\n", __FUNCTION__);
        return 0;
    }

    // if no rules, return directly
    if (is_webfilter_data_empty())
    {
        return 0;
    }

    doff = (tcph->doff * 4);
    data = (void *)tcph + doff;
    dlen = ntohs(ip_hdr(skb)->tot_len) - iph->ihl * 4 - doff;

    tail = data + dlen;
    if (dlen > 1024)
    {
        dlen = 1024;
        tail = data + 1024;
    }

    // POST / HTTP/1.0$$$$
    // GET / HTTP/1.0$$$$   minimum
    // 0123456789012345678
    //      9876543210
    if (((p = findend(data + 14, tail, 18)) == NULL) || (memcmp(p - 9, " HTTP/", 6) != 0))
    {
        return 0;
    }
    //LOG("===%s, find Request Line!!\n", __FUNCTION__);

    // save uri to buf
    uri_data = data + 4;
    //get_strip_string(WEBFILTER_STR_URI, &uri_data, p - 9, &uri_len);
    // support '?' in uri
    if (get_strip_uri(&uri_data, p - 9, &uri_len))
    {
        return 0;
    }
    LOG("urI len : %4d, urI str : ", uri_len);
    LOGSTR(uri_data, uri_len);

    spin_lock_bh(&webfilter_spin_lock);
    // gather host, referer..
    while (1)
    {
        // find all info, so break
        if (WEBFILTER_GET_ALL_OK == flag)
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
            const char * host_data = data + 6;
            get_strip_string(WEBFILTER_STR_HOST, &host_data, p, &host_len);

            if (host_len >= LEN_WEBFILTER_HOST)
            {
                printk("%s, host(size:%d) is too long!\n", __FUNCTION__, host_len);
                goto out;
            }
            host = http_info->host;
            memcpy(host, host_data, host_len);
            host[host_len] = '\0';
            LOG("Host len: %4d, Host str: %s\n", host_len, host);

            if (host_len + uri_len >= LEN_WEBFILTER_URL)
            {
                LOG("%s, url(size:%d) is too long!\n", __FUNCTION__, host_len + uri_len);
                goto out;
            }
            url = http_info->url;
            memcpy(url, host_data, host_len);
            memcpy(url + host_len, uri_data, uri_len);
            url[host_len + uri_len] = '\0';
            LOG("urL len : %4d, urL str : %s\n", host_len + uri_len, url);
            flag |= WEBFILTER_GET_URL_OK;
        }
        else if (0 == memcmp(data, "Referer: ", 9))
        {
            const char * refer_data = data + 9;

            // get referer
            get_strip_string(WEBFILTER_STR_REFER, &refer_data, p, &refer_len);

            if (refer_len >= LEN_WEBFILTER_REFER)
            {
                LOG("%s, ref(size:%d) is too long!\n", __FUNCTION__, refer_len);
                goto out;
            }
            if (refer_len > 0)
            {
                refer = http_info->refer;
                memcpy(refer, refer_data, refer_len);
                refer[refer_len] = '\0';
                LOG("ref len : %4d, referer : %s\n", refer_len, refer);
                flag |= WEBFILTER_GET_REFER_OK;
            }
            else
            {
                LOG("%s, ref(size<=0), stranger!\n", __FUNCTION__);
            }
        }
    }
    //LOG("%s, match flag=0x%0x\n", __FUNCTION__, flag);

    if (WEBFILTER_GET_REFER_OK & flag)
    {
        unsigned char md5_refer[MD5_HASH_LEN] = {0};
        md5_hash(refer, refer_len, md5_refer);
        LOG("ref url hash : ");
        LOGHEX(md5_refer, MD5_HASH_LEN);
        // sec warning page
        if (refer && 0 == memcmp(md5_refer, sysctl_http_match_warn_page, MD5_HASH_LEN))
        {
            // free resource and pass
            target = 0;
            goto out;
        }
    }

    if (WEBFILTER_GET_URL_OK & flag)
    {
        memset((void*)hash, 0, MD5_HASH_LEN);
        md5_hash(url, host_len + uri_len, hash);
        LOG("web url hash : ");
        LOGHEX(hash, MD5_HASH_LEN);
        node = webfilter_query_htable(WEBFILTER_LIST_URL, hash);
        if (NULL == node)
        {
            // HOST Match : to find host in HOST htable
            //LOG("%s, query host now == >\n", __FUNCTION__);
            //node = webfilter_query_host_list(WEBFILTER_LIST_HOST, host);
            node = webfilter_query_host_split_search(WEBFILTER_LIST_HOST, host);
            if (node)
            {
                web_filter_msg_cont * msg = &node->msg;
                web_filter_elem_data * elem = (web_filter_elem_data *)msg->data;

                if (WEBFILTER_RULE_AD_HOST == elem->type || WEBFILTER_RULE_AD_THIRD == elem->type)
                {
                    target = check_ad_filter(msg, http_info, flag);
                }
                else
                {
                    target = msg->target;
                }
                if (target > 0)
                {
                    update_webfilter_match_count(WEBFILTER_LIST_HOST, elem->type);
                }
            }
        }
        else
        {
            // store "target" action, node->msg.target
            target = node->msg.target;
            update_webfilter_match_count(WEBFILTER_LIST_URL, WEBFILTER_RULE_NONE);
        }
    }
out:
    spin_unlock_bh(&webfilter_spin_lock);
#if 0
    if (target)
    {
        LOG("http match debug switch : %d\n", sysctl_http_match_debug_switch);
        LOG("network: ip:0x%0x, mask:0x%0x\n", sysctl_http_match_network[0], sysctl_http_match_network[1]);
        LOG("refer page md5 is :");
        LOGHEX(sysctl_http_match_warn_page, MD5_HASH_LEN);
    }
#endif
    return target;
}
EXPORT_SYMBOL(web_filter_match);

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

int init_webfilter_match(void)
{
    int err = -1;

    err = md5_init();
    if (err)
    {
        printk(KERN_ERR"%s, init md5 error!\n", __FUNCTION__);
        goto error_md5;
    }
    err = alloc_web_filter_data();
    if (err)
    {
        printk(KERN_ERR"%s, alloc webfilter data error!\n",  __FUNCTION__);
        goto error_data;
    }

    err = init_netlink();
    if (err)
    {
        printk(KERN_ERR"%s, init netlink error!\n", __FUNCTION__);
        goto error_netlink;
    }

    err = init_web_filter_sysctl();
    if (err)
    {
        printk(KERN_ERR"init webfilter sysctl error!\n", __FUNCTION__);
        goto error_sysctl;
    }

    err = init_web_filter_mmap();
    if (err)
    {
        printk(KERN_ERR"init webfilter mmap error!\n", __FUNCTION__);
        goto error_mmap;
    }
    printk(KERN_INFO"%s, succeed!\n", __FUNCTION__);
    return 0;

error_mmap:
    fini_web_filter_sysctl();
error_sysctl:
    clean_netlink();
error_netlink:
    free_web_filter_data();
error_data:
    md5_cleanup();
error_md5:
    return err;
}
EXPORT_SYMBOL(init_webfilter_match);

void fini_webfilter_match(void)
{
    fini_web_filter_sysctl();
    fini_web_filter_mmap();
    clean_netlink();
    free_web_filter_data();
    md5_cleanup();
}
EXPORT_SYMBOL(fini_webfilter_match);

/*==================== webfilter match module ======================*/
#ifdef __WEBFILTER_MATCH_MODULE__

/* is_vaild_get_packet : is valid HTTP GET packet
 * return value : 0 or 1
 */
static int is_vaild_get_packet(const struct sk_buff * skb)
{
    const struct iphdr *iph = ip_hdr(skb);
    const struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
    const char *data = NULL;
    int doff = 0;
    int dlen = 0;
    __u32 sig;

    // skip non-http request, include syn/fin
    doff = (tcph->doff * 4);
    data = (void *)tcph + doff;
    dlen = ntohs(iph->tot_len) - iph->ihl * 4 - doff;

    // POST / HTTP/1.0$$$$
    // GET / HTTP/1.0$$$$
    // 1234567890123456789
    if (dlen < 18)
    {
        return 0;
    }
    //LOG("===%s, dlen=%d, tcph-len=%d, iph-len=%d\n", __FUNCTION__, dlen, doff, iph->ihl * 4);

    // Only match "GET " request
    sig = *(__u32 *)data;
    if (sig == constant_get_string)
    {
        return 1;
    }
    //LOG("===%s, match GET===\n", __FUNCTION__);

    return 0;
}

/* webfilter_mt : webfilter module match function 
 * return value : return target
 */
static bool webfilter_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
    int target = 0;
    const struct ipt_webfilter_info *info = par->matchinfo;
    const int offset = par->fragoff;

    if (offset != 0)
    {
        return 0;
    }

    switch (info->mode)
    {
    case IPT_WEB_HTTP:
    case IPT_WEB_HOST:
    case IPT_WEB_URI:
        return 0;
    case IPT_WEB_HORE:
        // entire request line and host line
        //LOG("%s, info->mode=HORE\n", __FUNCTION__);
        break;
    default:
        // shutup compiler
        return 0;
    }

    if (is_valid_get_packet(skb))
    {
        target = web_filter_match(skb);
    }

    return target ? target : 0;
}

static int webfilter_chk(const struct xt_mtchk_param *par)
{
    return 0;
}

static struct xt_match webfilter_match = {
    .name       = "webfilter",
    .family     = NFPROTO_IPV4,
    .match      = webfilter_mt,
    .matchsize  = sizeof(struct ipt_webfilter_info),
    .checkentry = webfilter_chk,
    .destroy    = NULL,
    .me         = THIS_MODULE
};

static int __init init_webfilter_module(void)
{
    int err = -1;
    err = init_webfilter_match();
    if (err)
    {
        return err;
    }

    err = xt_register_match(&webfilter_match);
    if (err)
    {
        fini_webfilter_match();
        return err;
    }
    return 0;
}

static void __exit fini_webfilter_module(void)
{
    xt_unregister_match(&webfilter_match);
    fini_webfilter_match();
}

module_init(init_webfilter_module);
module_exit(fini_webfilter_module);
MODULE_AUTHOR("miwifi-network");
MODULE_DESCRIPTION("web request filter module (experimental)");
MODULE_LICENSE("GPL");
#endif // __WEBFILTER_MATCH_MODULE__
