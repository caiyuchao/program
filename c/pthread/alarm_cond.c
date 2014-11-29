#include "csapp.h"

#define MAXL 100

typedef struct alarm_struct {
  struct alarm_struct *next;
  int seconds;
  time_t time;
  char mesg[MAXL];
} alarm_t;


pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t alarm_cond = PTHREAD_COND_INITIALIZER;

alarm_t *alarm_list = NULL;
time_t current_alarm = 0;

void alarm_insert(alarm_t *alarm);
void *alarm_thread(void * arg);


/* main function */
int main(int argc, void **argv){
  int status;
  time_t now;
  char line[MAXL];
  alarm_t alarm_tmp, *alarm;
  pthread_t tid;

  status = pthread_create(&tid, NULL, alarm_thread, NULL);
  if(status != 0)
    posix_error(status, "Create alarm thread");

  while(1){
    printf("Alarm> ");
    if(fgets(line, sizeof(line), stdin) == NULL)
      exit(0);
    now = time(NULL);
    if(strlen(line) <= 1) continue;
    if(sscanf(line, "%d %64[^\n]", alarm_tmp.seconds, alarm_tmp.mesg) < 2){
      fprintf(stderr, "Bad command!\n");
    } else{
      alarm = (alarm_t *)malloc(sizeof(alarm_t));
      if(alarm == NULL)
	unix_error("Allocate a alarm_t");
      alarm->seconds = alarm_tmp.seconds + now;
      strcpy(alarm->mesg, alarm_tmp.mesg);

      status = pthread_mutex_lock(&alarm_mutex);
      if(status != 0)
	posix_error(status, "Lock mutex");

      alarm_insert(alarm);

      status = pthread_mutex_unlock(&alarm_mutex);
      if(status != 0)
	posix_error(status, "Unlock mutex");  

    } /* end of else */

  } /* end of while(1) */

  return 0;
} /* end of main */



/* function alarm_insert, it inserts a alarm to the alarm list */
void alarm_insert(alarm_t *alarm){
  int status;
  time_t now;
  alarm_t *p, **q;

  q = &alarm_list;
  p = *q;
  while(p){
    if(p->time > alarm->time){
      alarm->next = p;
      *q = alarm;
      break;
    } else{
      q = &p->next;
      p = p->next;
    }
  }
  if(p == NULL){
    *q = alarm;
    alarm->next = NULL;
  }

#ifdef DEBUG
  now = time(NULL);
  printf("alarm list: ");
  for(p = alarm_list; p != NULL; p = p->next){
    printf("%d(%d)['%s']", p->time, p->time - now, p->mesg);
  }
#endif
    
  if(current_alarm == 0 || alarm->time < current_alarm){
    current_alarm = alarm->time;
    status = pthread_cond_signal(&alarm_cond);
    if(status != 0)
      posix_error(status, "Signal cond");
  }


} /* end of function alarm_insert */



/* function alarm_thread, it remove the latest alarm from alarm list and alarm it */
void *alarm_thread(void * arg){
  alarm_t *alarm;
  struct timespec cond_time;
  time_t now;
  int status, expired;

  status = pthread_mutex_lock(&alarm_mutex);
  if(status != 0)
    posix_error(status, "Lock mutex");  

  while(1){
    current_alarm = 0;
    while(alarm_list == NULL){
      status = pthread_cond_wait(&alarm_cond, &alarm_mutex);
      if(status != 0)
	posix_error(status, "Wait on cond");  
    }
    alarm = alarm_list;
    alarm_list = alarm_list->next;
    now = time(NULL);
    expired = 0;
    if(alarm->time > now){
#ifdef DEBUG
      printf("waiting: %d(%d)'%s'\n", alarm->time, alarm->time - now, alarm->mesg);
#endif
      cond_time.tv_sec = alarm->time;
      cond_time.tv_nsec = 0;
      current_alarm = alarm->time;
      while(current_alarm = alarm->time){
	status = pthread_cond_timedwait(&alarm_cond, &alarm_mutex, &cond_time);
	if(status == ETIMEDOUT){
	  expired = 1;
	  break;
	} else if(status != 0){
	  posix_error(status, "Cond timewait");
	}
      }	/* end of while(current_alarm ...) */
      if(!expired)
	alarm_insert(alarm);
    } /* end of if(alarm_time ...) */
    else expired = 1;

    if(expired){
      printf("(%d) %s\n", alarm->seconds, alarm->mesg);
      free(alarm);
    }
  }

  status = pthread_mutex_unlock(&alarm_mutex);
  if(status != 0)
    posix_error(status, "Unlock mutex");  
  

} /* end of function alarm_thread */
