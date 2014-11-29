#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct data_control {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} control;

void *thread_function(control *pctl) {
  int i;
  for ( i=0; i<20; i++) {
	pthread_mutex_lock(&(pctl->mutex));
  	printf("pthread_cond_wait start[%d]\n", i);
  	pthread_cond_wait(&(pctl->cond),&(pctl->mutex) );
  	printf("pthread_cond_wait done[%d]\n", i);
	pthread_mutex_unlock(&(pctl->mutex));
  }
  return NULL;
}

int main(void) {

  int x;
  control ctl;
  pthread_t mythread;

  /* CREATION */
  if(pthread_mutex_init(&ctl.mutex,NULL)){
	  printf("cond init error\n");
	  return 1;
  }
  if(pthread_cond_init(&ctl.cond,NULL)){
	  printf("cond init error\n");
	  return 1;
  }

  if ( pthread_create( &mythread, NULL, thread_function, &ctl) ) { 
    printf("error creating thread.\n");
    abort();
  }

  pthread_cond_signal(&ctl.cond);
  printf("pthread_cond_signal done\n");

  sleep(5);

  pthread_cond_signal(&ctl.cond);
  printf("pthread_cond_signal done\n");

  if ( pthread_join ( mythread, NULL ) ) { 
    printf("error joining thread.\n");
    abort();
  }

  pthread_cond_destroy(&ctl.cond);
  pthread_mutex_destroy(&ctl.mutex);
  
  return 0;

}
