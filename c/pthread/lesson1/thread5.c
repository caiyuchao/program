#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void *thread_function(void *arg) {
  int i;
  for ( i=0; i<2; i++) {
    printf("Thread says hi!\n");
    sleep(1);
  }
  pthread_exit(NULL);
}
int main(void) {
  pthread_t mythread;
  
  if ( pthread_create( &mythread, NULL, thread_function, NULL) ) {
    printf("error creating thread.\n");
    abort();
  }
  printf("wait joining thread.\n");
  if ( pthread_join ( mythread, NULL ) ) {
    printf("error joining thread.\n");
    abort();
  }
  printf("waited joining thread.\n");
    sleep(1);
  exit(0);
}
