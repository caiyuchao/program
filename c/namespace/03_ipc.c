/*
 * ipcmk -Q
 * ipcs -q
 * sudo ./03_clone_ipc
 * ipcs -q
 * yubo@yubo.org
 * 2015-09-28
 */
#define _GNU_SOURCE 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <stdio.h> 
#include <sched.h> 
#include <signal.h> 
#include <unistd.h> 

/* 定义一个给 clone 用的栈，栈大小1M */ 
#define STACK_SIZE (1024 * 1024) 
static char container_stack[STACK_SIZE]; 
char* const container_args[] = { "/bin/bash", NULL }; 


int container_main(void* arg) { 
	char **args = (char **)arg;

	printf("Container - inside the container!\n"); 
	if (sethostname("container", 10)){
		perror("sethostname");
	}

	execv(args[0], args); 
	printf("Something's wrong!\n"); 
	return 1; 
} 

int main() { 
	printf("Parent - start a container!\n"); 
	/* 调用clone函数，其中传出一个函数，还有一个栈空间的（为什么传尾指针，因为栈是反着的） */ 
	int container_pid = clone(container_main, container_stack+STACK_SIZE,
			SIGCHLD | CLONE_NEWUTS | CLONE_NEWIPC, (void *)container_args); 
	if(container_pid == -1){
		perror("clone");
	}
	/* 等待子进程结束 */ 
	waitpid(container_pid, NULL, 0); 
	printf("Parent - container stopped!\n"); 
	return 0; 
}
