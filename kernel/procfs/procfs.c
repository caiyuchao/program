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
#define proc_dirname "procfs"

#define PROCFS_MAX_SIZE 	1024
#define FILENAME_MAX_SIZE	32	
#define CODE_HASH_SIZE		64
#define CODE_FILE_DEF_SIZE	1024
#define CODE_FILE_MAX_SIZE	(1024*16)	// 16k

static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static struct proc_dir_entry *proc_dir, *proc_file;




ssize_t action_read(struct file *filp, char __user *user_buf,
		size_t count, loff_t *ppos);
ssize_t action_write(struct file *filp, const char __user *user_buf,
		size_t count, loff_t *ppos);

struct file_operations action_ops = {
	.owner = THIS_MODULE,
	.read = action_read,
	.write = action_write,
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

int init_module(void)
{

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
	remove_proc_entry(proc_filename, proc_dir);
	remove_proc_entry(proc_dirname, NULL);

	printk(KERN_INFO "/proc/%s/%s removed\n", proc_dirname, proc_filename);
}

