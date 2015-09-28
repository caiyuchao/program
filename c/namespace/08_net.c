/* 
 * sudo ./08_net
 *
 * yubo@yubo.org
 * 2015-09-28
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/capability.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)

static char container_stack[STACK_SIZE];
char* const container_args[] = { "/bin/bash", NULL };

int pipefd[2];

void set_map(char* file, int inside_id, int outside_id, int len) {
	FILE* mapfd = fopen(file, "w");
	if (NULL == mapfd) {
		perror("open file error");
		return;
	}
	fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);
	fclose(mapfd);
}

void set_uid_map(pid_t pid, int inside_id, int outside_id, int len) {
	char file[256];
	sprintf(file, "/proc/%d/uid_map", pid);
	set_map(file, inside_id, outside_id, len);
}

void set_gid_map(pid_t pid, int inside_id, int outside_id, int len) {
	char file[256];
	sprintf(file, "/proc/%d/gid_map", pid);
	set_map(file, inside_id, outside_id, len);
}


int container_main(void* arg) {
	char ch, **args = (char **)arg;

	printf("Container [%5d] - inside the container!\n", getpid());


	/* 等待父进程通知后再往下执行（进程间的同步） */
	close(pipefd[1]);
	read(pipefd[0], &ch, 1);

	printf("Container [%5d] - setup hostname!\n", getpid());

	//set hostname
	sethostname("container",10);
	//remount "/proc" to make sure the "top" and "ps" show container's information
	mount("proc", "/proc", "proc", 0, NULL);
	//setup network
	system("ip link set lo up");
	system("ip link set veth1 up");
	system("ip addr add 172.20.1.2/16 dev veth1");

	execv(args[0], args);
	printf("Something's wrong!\n");
	return 1;
}

int main()
{
	char *cmd;
	int container_pid;
	const int gid=getgid(), uid=getuid();

	printf("Parent: eUID = %ld;  eGID = %ld, UID=%ld, GID=%ld\n",
			(long) geteuid(), (long) getegid(), (long) getuid(), (long) getgid());

	pipe(pipefd);
	printf("Parent [%5d] - start a container!\n", getpid());
	container_pid = clone(container_main, container_stack+STACK_SIZE,
			CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWNET | SIGCHLD,
			(void *)container_args); 
	if(container_pid == -1){
		perror("clone");
	}

	//To map the uid/gid,
	//   we need edit the /proc/PID/uid_map (or /proc/PID/gid_map) in parent
	//The file format is
	//   ID-inside-ns   ID-outside-ns   length
	//if no mapping,
	//   the uid will be taken from /proc/sys/kernel/overflowuid
	//   the gid will be taken from /proc/sys/kernel/overflowgid
	set_uid_map(container_pid, 0, uid, 1);
	set_gid_map(container_pid, 0, gid, 1);

	asprintf(&cmd, "ip link set veth1 netns %d", container_pid);
	system("ip link delete veth0");
	system("ip link add veth0 type veth peer name veth1");
	system(cmd);
	system("ip link set veth0 up");
	system("ip addr add 172.20.1.1/16 dev veth0");
	free(cmd);
	printf("Parent [%5d] - Container [%5d]!\n", getpid(), container_pid);

	/* 通知子进程 */
	close(pipefd[1]);

	waitpid(container_pid, NULL, 0);
	printf("Parent - container stopped!\n");
	return 0;
}

