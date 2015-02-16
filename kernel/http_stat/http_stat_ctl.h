/*  
 *  http_stat.c
 *  yubo@yubo.org
 *  2015-02-16
 */
#if (PAGE_SIZE < 4096)
#error "http_stat needs at least a PAGE_SIZE of 4096"
#endif

#define CONFIG_HASH_SIZE	1024
#define CONFIG_HASH_MASK	(CONFIG_HASH_SIZE - 1)
#define CONFIG_PATH_SIZE	256
#define PROCFS_MAX_SIZE 	CONFIG_PATH_SIZE
#define PROC_FILENAME		"confdir"
#define PROC_DIRNAME		"http_stat"
#define CONFIG_TABLE_LOCK()	spin_lock_bh(&conf_table->lock)
#define CONFIG_TABLE_UNLOCK() spin_unlock_bh(&conf_table->lock)

struct config_table {
	spinlock_t  lock;
	char dirname[CONFIG_PATH_SIZE];
	struct hlist_head  head[CONFIG_HASH_SIZE];
};

struct config {
	struct hlist_node hnode;
	atomic_t __refcnt;
	char filename[CONFIG_PATH_SIZE];
	uint32_t code;
	char *content;
}____cacheline_aligned_in_smp;

#ifndef __HTTP_STAT_CTL_MODULE__
int http_stat_ctl_init(void);
void http_stat_ctl_exit(void);
struct config* config_search(uint32_t key);
#endif
