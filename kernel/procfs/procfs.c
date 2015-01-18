/*
 *  procfs
 *  yubo@yubo.org
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_ALERT */
#include <linux/proc_fs.h>   /* Needed for procfs */
#include <asm/uaccess.h>   /* Needed for copy_from_user */
#include <linux/types.h>

#define proc_filename "action"
#define proc_dirname "test_procfs"

#define PROCFS_MAX_SIZE 	1024
#define FILENAME_MAX_SIZE	32	
#define CODE_HASH_SIZE		64
#define CODE_FILE_DEF_SIZE	1024
#define CODE_FILE_MAX_SIZE	(1024*16)	// 16k

static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static struct proc_dir_entry *proc_dir, *proc_file;


struct code_table {
	spinlock_t  lock;
	struct hlist_head  head[CODE_HASH_SIZE];
};
static struct code_table *code_table;

struct code_node {
	struct hlist_node hnode;
	struct proc_dir_entry *proc_file;
	char filename[FILENAME_MAX_SIZE];
	uint32_t code;
	uint32_t len;
	uint32_t size;
	char *data;
};



ssize_t action_read(struct file *filp, char __user *user_buf,
		size_t count, loff_t *ppos);
ssize_t action_write(struct file *filp, const char __user *user_buf,
		size_t count, loff_t *ppos);
ssize_t code_node_read(struct file *filp, char __user *user_buf,
		size_t count, loff_t *ppos);
ssize_t code_node_write(struct file *filp, const char __user *user_buf,
		size_t count, loff_t *ppos);

struct file_operations action_ops = {
	.owner = THIS_MODULE,
	.read = action_read,
	.write = action_write,
};

struct file_operations code_node_ops = {
	.owner = THIS_MODULE,
	.read = code_node_read,
	.write = code_node_write,
};


#define __PROCFS_DEBUG__
#ifdef __PROCFS_DEBUG__
#define DEBUG_PRINT(fmt, a...) printk(fmt, ##a)
#else
#define DEBUG_PRINT(fmt, a...) do { } while(0)
#endif

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
	printk(KERN_INFO "procfile_write (/proc/%s/%s) called\n", proc_dirname, proc_filename);

	/* get buffer size */
	procfs_buffer_size = count;

	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	if (copy_from_user(procfs_buffer, user_buf, procfs_buffer_size)) {
		return -EFAULT;
	}

	return procfs_buffer_size;
}


ssize_t code_node_read(struct file *filp, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s/%s) called\n", proc_dirname, proc_filename);

	if (*ppos > 0) {
		/* we have finished to read, return 0 */
		ret = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(user_buf, procfs_buffer, procfs_buffer_size);
		ret = procfs_buffer_size;
		*ppos = procfs_buffer_size;
	}

	return ret;
}

uint32_t code_node_resize(struct code_node *cn)
{
	void *p;
	uint32_t size;

	size = cn->size ? cn->size : CODE_FILE_DEF_SIZE;
	p = krealloc(cn->data, cn->size + size, GFP_ATOMIC);
	if(p){
		cn->data = p;
		cn->size += size;
		return size;
	}else{
		printk(KERN_ERROR "krealloc fail ! \n");
		return 0;
	}
}

ssize_t code_node_write(struct file *filp, const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct code_node *cn;
	printk(KERN_INFO "procfile_write (/proc/%s/%s) called\n", proc_dirname, proc_filename);

	//find cn 
	
	if(count + ppos > CODE_FILE_MAX_SIZE)
		return 0;

	while(count + ppos > cn->size){
		if(!code_node_resize(cn))
			return 0;
	}

	/* get buffer size */
	procfs_buffer_size = count;

	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	if (copy_from_user(procfs_buffer, user_buf, procfs_buffer_size)) {
		return -EFAULT;
	}

	return procfs_buffer_size;
}

uint32_t code_hash(uint32_t code)
{
	return (code % CODE_HASH_SIZE);
}

struct code_node* __code_node_search(uint32_t code)
{
	struct hlist_node *pos;
	struct code_node *cn;
	uint32_t hash = code_hash(code);
	struct hlist_head *head = &code_table->head[hash];

	hlist_for_each_entry(cn, pos, head, hnode) {
		DEBUG_PRINT(KERN_DEBUG "%s:  hash:%u, head:%p, code_node:%p, pos:%p\n",
				__func__, hash, head, cn, pos);
		if (cn->code ==  code) {
			return cn;
		}
	}
	return NULL;
}

int __code_node_insert(struct code_node *cn)
{
	uint32_t hash = code_hash(cn->code);
	struct hlist_head *head = &code_table->head[hash];
	DEBUG_PRINT(KERN_DEBUG "%s:  hash:%u, head:%p, code_node:%p\n",
			__func__, hash, head, cn);
	hlist_add_head(&cn->hnode, head);

	return 0;
}

void __code_node_del(struct code_node *cn)
{
	hlist_del(&cn->hnode);
	return;
}

void __code_node_free(struct code_node *cn)
{
	kfree(cn);
}

#define CODE_TABLE_LOCK()	spin_lock_bh(&code_table->lock)
#define CODE_TABLE_UNLOCK() spin_unlock_bh(&code_table->lock)

struct code_node* code_node_search(uint32_t code)
{
	struct code_node* cn;
	CODE_TABLE_LOCK();
	cn = __code_node_search(code);
	CODE_TABLE_UNLOCK();

	return cn;
}


struct code_node* code_node_create(uint32_t code)
{
	struct code_node* cn;

	cn = kzalloc(sizeof(struct code_node), GFP_ATOMIC);
	if (cn == NULL) {
		printk(KERN_INFO "%s: zalloc failed code:%u\n",
				__func__, code);
		goto out;
	}
	cn->code = code;

	cn->data = NULL;
	cn->size = 0;

	CODE_TABLE_LOCK();
	if (__code_node_search(cn->code)) {
		CODE_TABLE_UNLOCK();
		printk(KERN_INFO "%s: duplicated code:%u\n",
				__func__, code);
		goto out_error;
	} else {
		__code_node_insert(cn);
		CODE_TABLE_UNLOCK();
	}

	snprintf(cn->filename, FILENAME_MAX_SIZE, "%u", code);
	cn->proc_file = proc_create(cn->filename, S_IRWXUGO, proc_dir, &code_node_ops);

	if (cn->proc_file == NULL) {
		remove_proc_entry(cn->filename, proc_dir);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s/%s\n",
				proc_dirname, cn->filename);
		goto out_error;
	}

	printk(KERN_INFO "%s created\n", cn->filename);

out:
	return cn;

out_error:
	kfree(cn->data);
	cn->data = NULL;
kmalloc_error:
	kfree(cn);
	cn = NULL;
	return cn;
}

struct code_node* code_node_del(uint32_t code)
{
	struct code_node* cn;

	CODE_TABLE_LOCK();
	cn = __code_node_search(code);
	if (cn){
		remove_proc_entry(proc_filename, proc_dir);
		__code_node_del(cn);
	}
	CODE_TABLE_UNLOCK();

	return cn;
}


int init_module(void)
{
	code_table = kzalloc(sizeof(struct code_table), GFP_ATOMIC);
	if (NULL == code_table) {
		return -ENOMEM;
	}

	proc_dir = proc_mkdir(proc_dirname, NULL);
	if (!proc_dir) {
		printk(KERN_ALERT "SetupProcDevice: could not create /proc/"
				proc_dirname "\n");
		return -EINVAL;
	}

	proc_file = proc_create(proc_filename, S_IRWXUGO, proc_dir, &action_ops);
	if (proc_file == NULL) {

		remove_proc_entry(proc_filename, proc_dir);
		remove_proc_entry(proc_dirname, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s/%s\n",
				proc_dirname, proc_filename);
		return -EINVAL;
	}

	return 0;
}

void cleanup_module(void)
{
	struct hlist_node *pos, *n;
	struct code_node *code_node;
	struct hlist_head *head;
	int i;

	remove_proc_entry(proc_filename, proc_dir);
	remove_proc_entry(proc_dirname, NULL);

	CODE_TABLE_LOCK();
	for (i=0; i<CODE_HASH_SIZE; i++) {
		head = &code_table->head[i];
		hlist_for_each_entry_safe(code_node, pos, n, head, hnode) {
			__code_node_del(code_node);
			__code_node_free(code_node);
		}
	}
	CODE_TABLE_UNLOCK();
	printk(KERN_INFO "/proc/%s/%s removed\n", proc_dirname, proc_filename);
}

