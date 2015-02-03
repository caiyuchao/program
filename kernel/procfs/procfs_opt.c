/*
 *  procfs_opt
 *  yubo@yubo.org
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_ALERT */
#include <linux/proc_fs.h>   /* Needed for procfs */
#include <asm/uaccess.h>   /* Needed for copy_from_user */
#include <linux/types.h>
#include <linux/sysctl.h>

static struct ctl_table_header *procfs_table_header;

static int procfs_int = 0;
static const int procfs_int_min = 0;
static const int procfs_int_max = 1;


int procfs_int_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int *valp = table->data;
	int val = *valp;
	int rc;

	rc = proc_dointvec(table, write, buffer, lenp, ppos);
	if (write && (*valp != val)) {
		if (*valp < 1 || !is_power_of_2(*valp)) {
			/* Restore the correct value */
			*valp = val;
		}
	}
	return rc;
}

/* /proc/sys as root */
struct ctl_table procfs_table[] = {
	{
		.procname = "procfs_int_handle",
		.data = &procfs_int,
		.maxlen = sizeof(int),
		.mode = 0644,
		.proc_handler = &procfs_int_handler,
	},
	{
		.procname = "procfs_int_minmax",
		.data = &procfs_int,
		.maxlen = sizeof(int),
		.mode = 0644,
		.proc_handler = proc_dointvec_minmax,
		.extra1 = &procfs_int_min,
		.extra2 = &procfs_int_max,
	},
	{
		.procname = "procfs_int",
		.data = &procfs_int,
		.maxlen = sizeof(int),
		.mode = 0644,
		.proc_handler = proc_dointvec,
	},
	{}
};


int procfs_sysfs_init(void)
{
	procfs_table_header = register_sysctl_table(procfs_table);
	if (procfs_table_header == NULL)
		return -EPERM;
	return 0;
}

void procfs_sysfs_exit(void)
{
	if (procfs_table_header)
		unregister_sysctl_table(procfs_table_header);
	return;
}


int init_module(void)
{

	procfs_sysfs_init();

	return 0;
}

void cleanup_module(void)
{
	procfs_sysfs_exit();
}

