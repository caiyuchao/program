/*  
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_ALERT */
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

struct file* file_open(const char* path, int flags, int rights) {
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


void file_close(struct file* file) {
    filp_close(file, NULL);
}


int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
} 

int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_sync(struct file* file) {
    vfs_fsync(file, file->f_path.dentry, 0);
    return 0;
}


struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen;
	char		d_name[1];
};

struct getdents_callback {
	struct linux_dirent __user * current_dir;
	struct linux_dirent __user * previous;
	int count;
	int error;
};

#define _MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * fs.h
 * DT_REG
 * DT_DIR
 */
static int readdir_callback(void * __buf, const char * name, int namlen, loff_t offset,
		   u64 ino, unsigned int d_type)
{
	char buff[1024], *p;

	if(d_type == DT_REG){
		p = buff;
		p += snprintf(p, sizeof(buff) - (p - buff), "filename[%d]:", namlen);
		p += snprintf(p, _MAX(sizeof(buff) - (p - buff), namlen + 1), "%s", name);
		//p += snprintf(p, sizeof(buff) - (p - buff), " inode:0%o d_type:0x%x", ino, d_type);
		printk("%s\n", buff);	
	}
	return 0;
}

static int list_dir(char *dirname)
{
	int ret;
    struct file* fp = NULL;

	fp = file_open(dirname, O_RDONLY, 0);
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
}

static read_file(char *filename)
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
}
int init_module(void)
{
    printk("Hello world\n");
	//read_file("/etc/hosts");
	list_dir("/tmp/test");
    return 0;
}

void cleanup_module(void)
{
    printk("Goodbye world\n");
}

