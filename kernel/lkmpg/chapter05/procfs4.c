/*
 *  procfs4.c -  create a "file" in /proc
 *	This program uses the seq_file library to manage the /proc file.
 */

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <linux/seq_file.h>	/* for seq_file */

#define PROCFS_NAME "iter"

/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 *	- the /proc file is read (first time)
 *	- after the function stop (end of sequence)
 *
 */
static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter = 0;

	/* beginning a new sequence ? */
	if ( *pos == 0 )
	{
		/* yes => return a non null value to begin the sequence */
		return &counter;
	}
	else
	{
		/* no => it's the end of the sequence, return end to stop reading */
		*pos = 0;
		return NULL;
	}
}


/**
 * This function is called after the beginning of a sequence.
 * It's called untill the return is NULL (this ends the sequence).
 *
 */
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}

/**
 * This function is called at the end of a sequence
 *
 */
static void my_seq_stop(struct seq_file *s, void *v)
{
	/* nothing to do, we use a static value in start() */
}


/**
 * This function is called for each "step" of a sequence
 *
 */
static int my_seq_show(struct seq_file *s, void *v)
{
	loff_t *spos = (loff_t *) v;

	seq_printf(s, "%Ld\n", *spos);
	return 0;
}

/**
 * This structure gather "function" to manage the sequence
 *
 */
static struct seq_operations my_seq_ops = {
	.start = my_seq_start,
	.next  = my_seq_next,
	.stop  = my_seq_stop,
	.show  = my_seq_show
};

/**
 * This function is called when the /proc file is open.
 *
 */
static int my_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
};

/**
 * This structure gather "function" that manage the /proc file
 *
 */
static struct file_operations my_file_ops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};


int init_module()
{
	/* 3.3 -> 3.13
	   entry = create_proc_entry(PROCFS_NAME, S_IFREG | S_IRUGO | S_IWUSR, NULL);
	   */
	struct proc_dir_entry *entry;
	entry = proc_create(PROCFS_NAME, S_IRWXUGO, NULL, &my_file_ops);

	if (entry == NULL) {
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

