/*
 *  procfs3.c -  create a "file" in /proc,use the file_operation way
 *  		to manage the file.
 *
 */

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <linux/sched.h>    /* for current_euid */
#include <asm/uaccess.h>	/* for copy_from_user */

#include "internal.h"

#define PROCFS_NAME "buffer2k"
#define PROCFS_MAX_SIZE 	2048

/**
 * The buffer (2k) for this module
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
	static int finished = 0;

	/*
	 * We return 0 to indicate end of file, that we have
	 * no more information. Otherwise, processes will
	 * continue to read from us in an endless loop.
	 */
	if ( finished ) {
		printk(KERN_INFO "procfs_read: END\n");
		finished = 0;
		return 0;
	}

	finished = 1;

	/*
	 * We use put_to_user to copy the string from the kernel's
	 * memory segment to the memory segment of the process
	 * that called us. get_from_user, BTW, is
	 * used for the reverse.
	 */
	if ( copy_to_user(user_buf, procfs_buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	printk(KERN_INFO "procfs_read: read %lu bytes\n", procfs_buffer_size);

	return procfs_buffer_size;	/* Return the number of bytes "read" */
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

/*
 * This function decides whether to allow an operation
 * (return zero) or not allow it (return a non-zero
 * which indicates why it is not allowed).
 *
 * The operation can be one of the following values:
 * 0 - Execute (run the "file" - meaningless in our case)
 * 2 - Write (input to the kernel module)
 * 4 - Read (output from the kernel module)
 *
 * This is the real function that checks file
 * permissions. The permissions returned by ls -l are
 * for referece only, and can be overridden here.
 */

static int module_permission(struct inode *inode, int op)
{
	/*
	 * We allow everybody to read from our module, but
	 * only root (uid 0) may write to it
	 */
	if (op == 4 || (op == 2 && uid_eq(current_euid() , GLOBAL_ROOT_UID)))
		return 0;

	/*
	 * If it's anything else, access is denied
	 */
	return -EACCES;
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

	/* error: assignment of member 'permission' in read-only object
	 * const struct inode_operations *proc_iops
	Our_Proc_File->proc_iops->permission = module_permission;
	*/

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

