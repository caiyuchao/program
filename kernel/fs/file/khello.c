/*  
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_ALERT */
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

static struct file* file_open(const char* path, int flags, int rights) {
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if(IS_ERR(filp)) {
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

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

static int read_file(char *filename)
{
	int ret;
	struct file *fp;
	unsigned long long offset;
	unsigned char buff[1024+1];
	fp = file_open(filename, O_RDWR|O_LARGEFILE, 0);
	if(fp){
		offset = 0;
		while((ret = file_read(fp, offset, buff, 1024))){
				buff[ret] = '\0';
				offset += ret;
				printk("%s", buff);
		}
		file_close(fp);
	}else{
		printk("file_open faild\n");
	}
	return 0;
}

int init_module(void)
{
    printk("Hello world\n");
	read_file("/etc/hosts");
    return 0;
}

void cleanup_module(void)
{
    printk("Goodbye world\n");
}

