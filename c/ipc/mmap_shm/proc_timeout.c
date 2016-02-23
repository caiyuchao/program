#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>

#define msleep(a) poll(NULL, 0, a)

typedef struct {
	size_t size;
	char *addr;
}shm_t;

typedef struct {
	pthread_t thr_wait, thr_timeout;
	pid_t pid;
	int timeout;
	int status;
}proc_wt_t;

#define PROC_S_EXITED 0
#define PROC_S_SIGNALED 1
#define PROC_S_TIMEOUT 2
#define PROC_S_ERR 4

int shm_alloc(shm_t *shm) {                  
	shm->addr = (char *) mmap(NULL, shm->size,
			PROT_READ|PROT_WRITE,
			MAP_ANON|MAP_SHARED, -1, 0);

	if (shm->addr == MAP_FAILED) {
		printf("mmap(MAP_ANON|MAP_SHARED, %zu) failed", shm->size);
		return -1;
	}              
	return 0; 
}                  


void shm_free(shm_t *shm){                  
	if (munmap((void *)shm->addr, shm->size) == -1) {
		printf("munmap(%p, %zu) failed", shm->addr, shm->size);
	}              
}

typedef void (*proc_pt)(shm_t *out, void *data);

static void kill_pid(void *arg)
{
	proc_wt_t *wt = (proc_wt_t *)arg;
	int s, i;

	wt->status |= PROC_S_TIMEOUT;
	s = kill(wt->pid, SIGTERM);
	printf("kill %d return:%d errno:%d\n", wt->pid, s, errno);
	for(i = 0; i < 100; i++){
		if ((s = kill(wt->pid, SIGTERM)) == -1){
			if(errno == ESRCH){
				return;
			}
		}
		msleep(10);
	}
	kill(wt->pid, SIGKILL);
}

static void * th_wait(void *arg)
{
	proc_wt_t *wt = (proc_wt_t *)arg;
	pid_t w;
	int status;

	//pthread_cleanup_push(kill_pid, wt);

	status = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (status != 0)
		printf("pthread_setcancelstate %d\n", status);

	do {
		w = waitpid(wt->pid, &status, WUNTRACED | WCONTINUED);
		if (w == -1) {
			wt->status |= PROC_S_ERR;
			perror("waitpid");
			goto out;
		}

		if (WIFEXITED(status)) {
			wt->status |= PROC_S_EXITED;
			printf("exited, status=%d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			wt->status |= PROC_S_SIGNALED;
			printf("killed by signal %d\n", WTERMSIG(status));
		} else if (WIFSTOPPED(status)) {
			printf("stopped by signal %d\n", WSTOPSIG(status));
		} else if (WIFCONTINUED(status)) {
			printf("continued\n");
		}
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));

out:
	pthread_cancel(wt->thr_timeout);
	//pthread_cleanup_pop(0);

	return NULL;
}

static void * th_timeout(void *arg)
{
	proc_wt_t *wt = (proc_wt_t *)arg;
	msleep(wt->timeout);
	printf("timeout %d and cancle wait_pid\n", wt->timeout);
	wt->status |= PROC_S_TIMEOUT;
	kill_pid(wt);
	//pthread_cancel(wt->thr_wait);
	return NULL;
}

int wait_timeout(pid_t pid, int timeout){
	proc_wt_t wt;
	int s;

	wt.pid = pid;
	wt.timeout = timeout;
	wt.status = 0;

	s = pthread_create(&wt.thr_wait, NULL, &th_wait, &wt);
	if (s != 0){
		printf("pthread_create %d\n", s);
		goto out;
	}

	s = pthread_create(&wt.thr_timeout, NULL, &th_timeout, &wt);
	if (s != 0){
		printf("pthread_create %d\n", s);
		pthread_cancel(wt.thr_wait);
		goto out;
	}

	pthread_join(wt.thr_wait, NULL);
	pthread_join(wt.thr_timeout, NULL);

out:
	return -wt.status;
}
/*
static void sig_term(int signo)
{       
	printf("term and exit %d\n", getpid());
	exit(1);
}
*/

int process(proc_pt proc, shm_t *shm, void *arg, int timeout){
	pid_t pid;

	pid = fork();

	switch(pid){
		case -1:
			return -1;
		case 0:
			/*
			if(signal(SIGTERM,sig_term) == SIG_ERR){
				printf("signal error\n");
				exit(1);
			}
			*/
			proc(shm, arg);
			exit(0);
			break;
		default:
			printf("children pid:%d\n", pid);
			return wait_timeout(pid, timeout);
	}
	return 0;
}

void proc_handle(shm_t *out, void *data)
{
	int i, j, runtime;
	i = snprintf(out->addr, out->size, "hello");
	runtime = *(int *)data;
	for(j = 0; j < runtime/100; j++){
		msleep(100);
		printf("from child %d\n", j);
	}
	i += snprintf(out->addr+i, out->size-i, ",world!");
}

int main(){
	shm_t buf;
	int s;
	int runtime, timeout;

	buf.size = 2048;
	shm_alloc(&buf);
	buf.addr[0] = 0;
	runtime = 2000;
	timeout = 1000;

	s = process(proc_handle, &buf, &runtime, timeout);
	printf("status[%d] shm.addr[%s]\n", s, buf.addr);

	shm_free(&buf);
}
