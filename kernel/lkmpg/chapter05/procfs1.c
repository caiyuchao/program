/*
 *  procfs1.c -  create a "file" in /proc/test
 *
 */

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */

#define procfs_name "helloworld"
#define PROC_DIRNAME "test"

/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *ProcEntryDir, *ProcEntryFile;

/* Put data into the proc fs file.
 *
 * Arguments
 * =========
 * 1. The buffer where the data is to be inserted, if
 *    you decide to use it.
 * 2. A pointer to a pointer to characters. This is
 *    useful if you don't want to use the buffer
 *    allocated by the kernel.
 * 3. The current position in the file
 * 4. The size of the buffer in the first argument.
 * 5. Write a "1" here to indicate EOF.
 * 6. A pointer to data (useful in case one common
 *    read for multiple /proc/... entries)
 *
 * Usage and Return Value
 * ======================
 * A return value of zero means you have no further
 * information at this time (end of file). A negative
 * return value is an error condition.
 *
 * For More Information
 * ====================
 * The way I discovered what to do with this function
 * wasn't by reading documentation, but by reading the
 * code which used it. I just looked to see what uses
 * the get_info field of proc_dir_entry struct (I used a
 * combination of find and grep, if you're interested),
 * and I saw that  it is used in <kernel source
 * directory>/fs/proc/array.c.
 *
 * If something is unknown about the kernel, this is
 * usually the way to go. In Linux we have the great
 * advantage of having the kernel source code for
 * free - use it.
 */

int procfile_read(struct file *filp, const char __user *buf,
                           size_t count, loff_t *ppos);

struct file_operations ControlFileOps = {
	.owner = THIS_MODULE,
	.read = procfile_read,
};



int procfile_read(struct file *filp, const char __user *buf,
                           size_t count, loff_t *ppos)
{
	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_name);

	/*
	 * We give all of our information in one go, so if the
	 * user asks us if we have more information the
	 * answer should always be no.
	 *
	 * This is important because the standard read
	 * function from the library would continue to issue
	 * the read system call until the kernel replies
	 * that it has no more information, or until its
	 * buffer is filled.
	 */
	if (*ppos > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		ret = sprintf(buf, "HelloWorld!\n");
		*ppos += ret;
	}

	return ret;
}

int init_module()
{
	/* Create /proc/fs/vmblock */
	ProcEntryDir = proc_mkdir(PROC_DIRNAME, NULL);
	if (!ProcEntryDir) {
		printk(KERN_ALERT "SetupProcDevice: could not create /proc/"
				PROC_DIRNAME "\n");
		return -EINVAL;
	}

	/* 3.3 -> 3.13
	   Our_Proc_File = create_proc_entry(procfs_name, S_IFREG | S_IRUGO | S_IWUSR, NULL);
	   */
	ProcEntryFile = proc_create(procfs_name, S_IFREG | S_IRUGO | S_IWUSR, ProcEntryDir, &ControlFileOps);

	if (ProcEntryFile == NULL) {

		remove_proc_entry(procfs_name, ProcEntryDir);
		remove_proc_entry(PROC_DIRNAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s/%s\n",
				PROC_DIRNAME, procfs_name);
		return -EINVAL;
	}

	printk(KERN_INFO "/proc/%s/%s created\n", PROC_DIRNAME, procfs_name);
	return 0;	/* everything is ok */
}

void cleanup_module()
{
	/*
	   remove_proc_entry(procfs_name, &proc_root);
	   */
	remove_proc_entry(procfs_name, ProcEntryDir);
	remove_proc_entry(PROC_DIRNAME, NULL);
	printk(KERN_INFO "/proc/%s/%s removed\n", PROC_DIRNAME, procfs_name);
}

