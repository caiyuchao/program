/* 
 * curdir tree should like 06_rootfs.list
 * usage: sudo ./06_chroot
 * yubo@yubo.org
 * 2015-09-28
 */
#define _GNU_SOURCE 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/mount.h> 
#include <stdlib.h>
#include <stdio.h> 
#include <sched.h> 
#include <signal.h> 
#include <unistd.h> 

/*
 * ipcmk -Q
 * ipcs -q
 * sudo ./03_clone_ipc
 * ipcs -q
 */
/* 定义一个给 clone 用的栈，栈大小1M */ 
#define STACK_SIZE (1024 * 1024) 
static char container_stack[STACK_SIZE]; 
char* const container_args[] = { "/bin/bash", "-l", NULL }; 


int container_main(void* arg) { 
	char **args = (char **)arg;

	printf("Container [%5d] - inside the container!\n", getpid());
	if (sethostname("container", 10)){
		perror("sethostname");
	}

	//remount "/proc" to make sure the "top" and "ps"
	//show container's information
	if(mount("proc",   "rootfs/proc",    "proc",  0, NULL)) perror("proc");
	if(mount("sysfs",  "rootfs/sys",     "sysfs", 0, NULL)) perror("sys");
	if(mount("none",   "rootfs/tmp",     "tmpfs",   0, NULL)) perror("tmp");
	if(mount("udev",   "rootfs/dev",     "devtmpfs", 0, NULL)) perror("dev");
	if(mount("tmpfs",  "rootfs/run",     "tmpfs", 0, NULL)) perror("run");
	if(mount("shm",    "rootfs/dev/shm", "tmpfs", 0, NULL)) perror("dev/shm");
	if(mount("devpts", "rootfs/dev/pts", "devpts", 0, NULL)) perror("dev/pts");

	/* 
	 * 模仿Docker的从外向容器里mount相关的配置文件 
	 * 你可以查看：/var/lib/docker/containers/<container_id>/目录， 
	 * 你会看到docker的这些文件的。 
	 */
	if(mount("conf/hosts", "rootfs/etc/hosts", "none", MS_BIND, NULL) || 
			mount("conf/hostname", "rootfs/etc/hostname", "none", MS_BIND, NULL) ||
			mount("conf/resolv.conf", "rootfs/etc/resolv.conf", "none", MS_BIND, NULL)){
		perror("conf");
	}

	/* 模仿docker run命令中的 -v, --volume=[] 参数干的事 */
	if(mount("/tmp/t1", "rootfs/mnt", "none", MS_BIND, NULL)) perror("mnt");

	/* chroot */
	if(chdir("./rootfs") || chroot("./")){
		perror("chdir/chroot");
	}
	if(execv(args[0], args)) perror("exec");
	printf("Something's wrong!\n"); 
	return 1; 
} 

int main() { 
	printf("Parent - start a container!\n"); 
	/* 启用Mount Namespace - 增加CLONE_NEWNS参数 */
	int container_pid = clone(container_main, container_stack+STACK_SIZE,
			SIGCHLD | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS,
			(void *)container_args); 
	if(container_pid == -1){
		perror("clone");
	}
	/* 等待子进程结束 */ 
	waitpid(container_pid, NULL, 0); 
	printf("Parent - container stopped!\n"); 
	return 0; 
}
