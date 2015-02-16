/*  
 *  http_stat.c
 *  yubo@yubo.org
 *  2015-02-16
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_ALERT */
#include <linux/proc_fs.h>   /* Needed for procfs */
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

#if (PAGE_SIZE < 4096)
#error "http_stat needs at least a PAGE_SIZE of 4096"
#endif

#define CONFIG_HASH_SIZE	1024
#define CONFIG_HASH_MASK	(CONFIG_HASH_SIZE - 1)
#define CONFIG_PATH_SIZE	256
#define PROCFS_MAX_SIZE 	CONFIG_PATH_SIZE
#define proc_filename		"confdir"
#define proc_dirname		"http_stat"
#define CONFIG_TABLE_LOCK()	spin_lock_bh(&conf_table->lock)
#define CONFIG_TABLE_UNLOCK() spin_unlock_bh(&conf_table->lock)

static char * read_file(const char *filename);
static int reload_confdir(char *confdir);
ssize_t action_read(struct file *filp, char __user *user_buf,
		size_t count, loff_t *ppos);
ssize_t action_write(struct file *filp, const char __user *user_buf,
		size_t count, loff_t *ppos);


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

struct file_operations action_ops = {
	.owner = THIS_MODULE,
	.read = action_read,
	.write = action_write,
};

static struct config_table *conf_table;
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;
static struct proc_dir_entry *proc_dir, *proc_file;

ssize_t action_read(struct file *filp, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s/%s) called\n", proc_dirname, proc_filename);

	if (*ppos > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(user_buf, procfs_buffer, procfs_buffer_size);
		ret = procfs_buffer_size;
		*ppos = procfs_buffer_size;
	}

	return ret;
}

ssize_t action_write(struct file *filp, const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	int len;
	char *p;

	printk(KERN_INFO "procfile_write (/proc/%s/%s) called\n", 
			proc_dirname, proc_filename);

	if (count >= CONFIG_PATH_SIZE)
		return -EINVAL;

	if (copy_from_user(procfs_buffer, user_buf, count)) {
		return -EFAULT;
	}

	p = procfs_buffer;
	for(len = 0, p = procfs_buffer; len < count; len++, p++) {
		if (*p == 0 || *p == '\n'){
			break;
		}
	}
	procfs_buffer[len] = 0;
	procfs_buffer_size = len;
	printk("procfile_write %s\n", procfs_buffer);
	reload_confdir(procfs_buffer);

	return count;
}


static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)){
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}


static void file_close(struct file* file) {
	filp_close(file, NULL);
}

static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
} 

static uint32_t config_hash(uint32_t hash)
{
	/* fnv32 hash */
#if 0
	uint32_t hash = 2166136261U;
	for (; *s; s++)
		hash = (hash ^ *s) * 0x01000193;
#endif
	return (hash % CONFIG_HASH_MASK);
}

struct config* __config_search(uint32_t key)
{
	struct hlist_node *pos;
	struct config *conf;
	uint32_t hash = config_hash(key);
	struct hlist_head *head = &conf_table->head[hash];

	hlist_for_each_entry(conf, pos, head, hnode) {
		printk(KERN_DEBUG "%s:  hash:%d, head:%p, conf:%p, pos:%p\n",
				__func__, hash, head, conf, pos);
		if (conf->code == key) {
			printk(KERN_DEBUG "%s: [FIND %u] hash:%d\n",
					__func__, key, hash);
			return conf;
		}
	}
	return NULL;
}

int __config_insert(struct config *conf)
{
	uint32_t hash = config_hash(conf->code);
	struct hlist_head *head = &conf_table->head[hash];
	printk(KERN_DEBUG "%s:  hash:%d, head:%p, conf:%p\n",
			__func__, hash, head, conf);
	hlist_add_head(&conf->hnode, head);

	return 0;
}

void __config_del(struct config *conf)
{
	hlist_del(&conf->hnode);
	return;
}

void __config_free(struct config *conf)
{
	char *p;
	if(conf->content){
		p = conf->content;
		printk("free filename[%s] content[%s]\n", conf->filename, conf->content);
		for(p = conf->content; *p; p++){
			printk("%c[%d]", *p, *p);
		}

		kfree(conf->content);
	}
	kfree(conf);
}


void put_config(struct config *conf)
{
	return atomic_dec(&conf->__refcnt);
}

void get_config(struct config *conf)
{
	return atomic_inc(&conf->__refcnt);
}

struct config* config_search(uint32_t key)
{
	struct config *conf;
	CONFIG_TABLE_LOCK();
	conf = __config_search(key);
	if (conf)
		get_config(conf);
	CONFIG_TABLE_UNLOCK();

	return conf;
}

static void config_refresh(void)
{
	int i;
	struct config *conf;
	struct hlist_head *head;
	struct hlist_node *pos, *n;

	printk("config_refresh enter\n");
	CONFIG_TABLE_LOCK();
	for (i=0; i<CONFIG_HASH_SIZE; i++) {
		head = &conf_table->head[i];
		hlist_for_each_entry_safe(conf, pos, n, head, hnode) {
			if(conf->content){
				kfree(conf->content);
			}
			conf->content = read_file(conf->filename);
			if(conf->content == NULL){
				printk(KERN_INFO "%s:%d: zalloc failed filename:%s content\n",
						__func__, __LINE__, conf->filename);
				__config_del(conf);
				__config_free(conf);
			}
		}
	}
	CONFIG_TABLE_UNLOCK();



}

struct config* config_create(const char *filename)
{
	struct config* conf;

	printk("config_create enter\n");
	conf = kzalloc(sizeof(struct config), GFP_ATOMIC);
	if (conf == NULL) {
		printk(KERN_INFO "%s:%d: zalloc failed filename:%s\n",
				__func__, __LINE__, filename);
		return NULL;
	}
	strncpy(conf->filename, filename, CONFIG_PATH_SIZE);
	atomic_set((&conf->__refcnt), 1);
	conf->code = simple_strtoul(filename, NULL, 10);
	conf->content = NULL;

	CONFIG_TABLE_LOCK();
	if (__config_search(conf->code)) {
		CONFIG_TABLE_UNLOCK();
		printk(KERN_INFO "%s: filename:%s, but code[%u] already insert\n",
				__func__, filename, conf->code);
		kfree(conf);
		conf = NULL;
	} else {
		__config_insert(conf);
		CONFIG_TABLE_UNLOCK();
	}

	return conf;
}

struct config* config_del(uint32_t key)
{
	struct config* conf;

	CONFIG_TABLE_LOCK();
	conf = __config_search(key);
	if (conf)
		__config_del(conf);
	CONFIG_TABLE_UNLOCK();

	return conf;
}



/*
 * fs.h
 * DT_REG
 * DT_DIR
 */
static int readdir_callback(void * __buf, const char * name, int namlen, loff_t offset,
		u64 ino, unsigned int d_type)
{
	printk("readdir_callback enter \n");
	if(d_type == DT_REG){
		if(simple_strtoul(name, NULL, 10))
			config_create(name);
	}
	return 0;
}

static int load_dir(char *dirname)
{
	int ret;
	struct file* fp = NULL;


	printk("load_dir enter\n");
	if(strlen(dirname) > CONFIG_PATH_SIZE)
		return 1;
	strncpy(conf_table->dirname, dirname, CONFIG_PATH_SIZE);

	fp = file_open(dirname, O_RDONLY | O_LARGEFILE, 0);
	if(!fp){
		printk(KERN_ALERT "filp_open fail\n");
		return 1;
	}

	ret = vfs_readdir(fp, readdir_callback, NULL);
	if(ret){
		printk(KERN_ALERT "vfs readdir error %d\n", ret);
	}else{
		printk(KERN_ALERT "vfs readdir ok \n");
	}
	file_close(fp);
	config_refresh();
	return 0;
}

static char * buff_resize(void *fp, uint32_t resize){
	void *p;
	p = krealloc(fp, resize, GFP_ATOMIC);
	if(p){
		return p;
	}else{
		kfree(fp);
		return NULL;
	}

}

static char * read_file(const char *filename)
{
	int ret;
	struct file *fp;
	unsigned long long offset;
	uint32_t size = PAGE_SIZE;
	char *buff;
	char filepath[CONFIG_PATH_SIZE];

	printk("read_file(%s) enter\n", filename);
	snprintf(filepath, CONFIG_PATH_SIZE, "%s/%s", conf_table->dirname, filename);

	if((buff = kmalloc(size, GFP_ATOMIC)) == NULL){
		return NULL;
	}
	buff[0] = '\0';

	fp = file_open(filepath, O_RDONLY, 0);
	if(fp){
		offset = 0;
		while((ret = file_read(fp, offset, buff, 1024))){
			offset += ret;
			buff[offset] = '\0';
			printk("%s", buff);
			if(size - offset < 1024){
				size += PAGE_SIZE;
				if((buff = buff_resize(buff, size)) == NULL){
					file_close(fp);
					return NULL;
				}
			}
		}
		file_close(fp);
		return buff;
	}else{
		printk("file_open faild\n");
	}
	return NULL;
}

static int reload_confdir(char *confdir)
{
	int i;
	struct config *conf;
	struct hlist_head *head;
	struct hlist_node *pos, *n;

	printk("reload_confdir enter\n");
	CONFIG_TABLE_LOCK();
	for (i=0; i<CONFIG_HASH_SIZE; i++) {
		head = &conf_table->head[i];
		hlist_for_each_entry_safe(conf, pos, n, head, hnode) {
			__config_del(conf);
			__config_free(conf);
		}
	}
	CONFIG_TABLE_UNLOCK();
	return load_dir(confdir);
}

static int __init http_stat_init(void)
{
	int i, err = 0;

	conf_table = kzalloc(sizeof(struct config_table), GFP_ATOMIC);
	if (NULL == conf_table) {
		err = -ENOMEM;
		goto err_malloc;
	}
	spin_lock_init(&conf_table->lock);
	for (i = 0; i < CONFIG_HASH_SIZE; i++) {
		INIT_HLIST_HEAD(&conf_table->head[i]);
	}

	printk("Hello world, page_size %lu\n", PAGE_SIZE);

	proc_dir = proc_mkdir(proc_dirname, NULL);
	if (!proc_dir) {
		printk(KERN_ALERT "SetupProcDevice: could not create /proc/"
				proc_dirname "\n");
		goto err_out;
	}

	proc_file = proc_create(proc_filename, S_IRWXUGO, proc_dir, &action_ops);
	if (proc_file == NULL) {
		remove_proc_entry(proc_filename, proc_dir);
		remove_proc_entry(proc_dirname, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s/%s\n",
				proc_dirname, proc_filename);
		goto err_out;
	}
	return 0;

err_out:
	kfree(conf_table);
err_malloc:
	return -EINVAL;
}

static void __exit http_stat_exit(void)
{
	struct config *conf;
	struct hlist_head *head;
	struct hlist_node *pos, *n;
	int i;

	remove_proc_entry(proc_filename, proc_dir);
	remove_proc_entry(proc_dirname, NULL);
	CONFIG_TABLE_LOCK();
	for (i=0; i<CONFIG_HASH_SIZE; i++) {
		head = &conf_table->head[i];
		hlist_for_each_entry_safe(conf, pos, n, head, hnode) {
			__config_del(conf);
			__config_free(conf);
		}
	}
	CONFIG_TABLE_UNLOCK();
	kfree(conf_table);
	printk("Goodbye world\n");
}

module_init(http_stat_init);
module_exit(http_stat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yu Bo <yubo@yubo.org>");
