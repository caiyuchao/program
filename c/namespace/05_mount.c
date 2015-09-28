/*
 * usage: sudo ./05_mount
 * yubo@yubo.org
 * 2015-09-28
 */
#define _GNU_SOURCE 
#include <sys/types.h> 
#include <sys/wait.h> 
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
char* const container_args[] = { "/bin/bash", NULL }; 


int container_main(void* arg) { 
	char **args = (char **)arg;

	printf("Container [%5d] - inside the container!\n", getpid());
	if (sethostname("container", 10)){
		perror("sethostname");
	}
	/* 重新mount proc文件系统到 /proc下 */ 
	system("mount -t proc proc /proc");
	execv(args[0], args); 
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
