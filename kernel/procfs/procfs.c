/*
 *  procfs
 *  yubo@yubo.org
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_ALERT */

#define proc_filename "test"
#define proc_dirname "test_procfs"

#define PROCFS_MAX_SIZE 	1024
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static struct proc_dir_entry *proc_dir, *proc_file;


int procfile_read(struct file *filp, const char __user *buf,
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

/**
 * This function is called with the /proc file is written
 *
 */
ssize_t procfile_write(struct file *filp, const char __user *user_buf,
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

int procfile_create(char *file_name)
{
	/* 3.3 -> 3.13
	   Our_Proc_File = create_proc_entry(PROCFS_NAME, S_IFREG | S_IRUGO | S_IWUSR, NULL);
	   */
	proc_file = proc_create(file_name, S_IRWXUGO, NULL, &ControlFileOps);

	if (proc_file == NULL) {
		remove_proc_entry(file_name, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME);
		return -EINVAL;
	}

	printk(KERN_INFO "%s created\n", file_name);
	return 0;	/* everything is ok */
}

int init_module(void)
{
	proc_dir = proc_mkdir(proc_dirname, NULL);
	if (!proc_dir) {
		printk(KERN_ALERT "SetupProcDevice: could not create /proc/"
				proc_dirname "\n");
		return -EINVAL;
	}

	proc_file = proc_create(proc_filename, S_IFREG | S_IRUGO | S_IWUSR, ProcEntryDir, &ControlFileOps);
	if (proc_file == NULL) {

		remove_proc_entry(proc_filename, proc_dir);
		remove_proc_entry(proc_dirname, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s/%s\n",
				proc_dirname, proc_filename);
		return -EINVAL;
	}
    /*
     * A non 0 return means init_module failed; module can't be loaded.
     */
    return 0;
}

void cleanup_module(void)
{
	remove_proc_entry(proc_filename, proc_dir);
	remove_proc_entry(proc_dirname, NULL);
	printk(KERN_INFO "/proc/%s/%s removed\n", proc_dirname, proc_filename);
}

