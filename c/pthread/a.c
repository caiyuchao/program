#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define    NUM_THREADS     8

void *PrintHello(void *args)
{
	int thread_arg;
	pid_t p;
	int status;
	char *argv[] = {"hello"};
	p = fork();
	switch(p){
		case -1:
			return NULL;
		case 0:
			execvp("./a.sh", argv);
			exit(127);
			break;
		default:
			if(waitpid(p, &status,0)==-1)
				return NULL;
			return NULL;
	}
	//sleep(1);
	thread_arg = (int)args;
	printf("Hello from thread %d\n", thread_arg);
	return NULL;
}

int main(void)
{
	int rc,t;
	pthread_t thread[NUM_THREADS];

	for(t=0;;t++)
	{
		printf("Creating thread %d\n", t);
		//rc = pthread_create(&thread[t], NULL, PrintHello, (void *)t);
		rc = pthread_create(&thread[0], NULL, PrintHello, (void *)t);
		if (rc)
		{
			printf("ERROR; return code is %d\n", rc);
			return EXIT_FAILURE;
		}
		sleep(1);
	}
	return EXIT_SUCCESS;
}
