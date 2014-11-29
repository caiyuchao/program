/*
 *  procfs2.c -  create a "file" in /proc
 *
 */

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */

#define PROCFS_NAME "buffer1k"
#define PROCFS_MAX_SIZE 	1024

/**
 * The buffer (1k) for this module
 *
 */
static char procfs_buffer[PROCFS_MAX_SIZE];

/**
 * The size of the data hold in the buffer
 *
 */
static unsigned long procfs_buffer_size = 0;

/**
 * The structure keeping information about the /proc file
 *
 */
static struct proc_dir_entry *Our_Proc_File;


ssize_t procfile_read(struct file *filp, char __user *user_buf,
                           size_t count, loff_t *ppos);

ssize_t procfile_write(struct file *filp, const char __user *user_buf,
                           size_t count, loff_t *ppos);

struct file_operations ControlFileOps = {
	.owner = THIS_MODULE,
	.read = procfile_read,
	.write = procfile_write,
};



/**
 * This function is called then the /proc file is read
 *
 */
ssize_t procfile_read(struct file *filp, char __user *user_buf,
                           size_t count, loff_t *ppos)
{
	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s) called\n", PROCFS_NAME);

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

/**
 * This function is called with the /proc file is written
 *
 */
ssize_t procfile_write(struct file *filp, const char __user *user_buf,
                           size_t count, loff_t *ppos)
{
	printk(KERN_INFO "procfile_write (/proc/%s) called\n", PROCFS_NAME);

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

int init_module()
{
	/* 3.3 -> 3.13
	   Our_Proc_File = create_proc_entry(PROCFS_NAME, S_IFREG | S_IRUGO | S_IWUSR, NULL);
	   */
	Our_Proc_File = proc_create(PROCFS_NAME, S_IRWXUGO, NULL, &ControlFileOps);

	if (Our_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME);
		return -EINVAL;
	}

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;	/* everything is ok */
}

void cleanup_module()
{
	/*
	 * remove_proc_entry(PROCFS_NAME, &proc_root);
	**/
	remove_proc_entry(PROCFS_NAME, NULL);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

