#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <signal.h> 

static void sig_alrm(int signo)
{       
	printf("alarm and exit\n");
	exit(127);
}

int main(int argc, const char *argv[])
{
	pid_t p;
	int status;
	p = fork();
	switch(p){
		case -1:
			return 1;
		case 0:
			if(signal(SIGALRM,sig_alrm) == SIG_ERR){
				printf("signal error\n");
				return -1; 
			}
			alarm(2);
			sleep(10);
			printf("sleep done and exit\n");
			exit(127);
			break;
		default:
			if(waitpid(p, &status,0) == -1){
				goto out;
			}
			goto out;
	}
out:
	printf("out\n");
	return 0;
}
