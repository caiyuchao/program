#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>


#define MSGSZ	 128
#define MAXLINE	 100000


/*
 * Declare the message structure.
 */

typedef struct msgbuf {
	long	mtype;
	char	mtext[MSGSZ];
} message_buf;

static FILE *fp;


int main()
{
	int msqid, i;
	key_t key;
	message_buf  sbuf;
	char buff[1024];
	struct timeval t0, t1, t2, t3;
	float timeuse;

	fp = fopen("demo.log","a+");
	gettimeofday(&t0,NULL);
	for (i=0; i<MAXLINE; i++){
		sprintf(buff,"fwrite : number %d\n",i);
		fputs(buff, fp);
	}
	fclose(fp);

	gettimeofday(&t1,NULL);
	for (i=0; i<MAXLINE; i++){
		fp = fopen("demo.log","a+");
		sprintf(buff,"fwrite : number %d\n",i);
		fputs(buff, fp);
		fclose(fp);
	}

	gettimeofday(&t2,NULL);
	key = 1234;
	if ((msqid = msgget(key, 0666)) < 0) {
		printf("msgget error \n");
		return 1;
	}

	sbuf.mtype=1;
	for (i=0; i<MAXLINE; i++){
		sprintf(sbuf.mtext,"mq : number %d\n",i);
		msgsnd(msqid, &sbuf, strlen(sbuf.mtext), IPC_NOWAIT);
		

	}
	gettimeofday(&t3,NULL);


    	timeuse=1000000*(t1.tv_sec-t0.tv_sec)+t1.tv_usec-t0.tv_usec;
        timeuse/=1000000;
	printf("%f\t", timeuse);

    	timeuse=1000000*(t2.tv_sec-t1.tv_sec)+t2.tv_usec-t1.tv_usec;
        timeuse/=1000000;
	printf("%f\t", timeuse);

    	timeuse=1000000*(t3.tv_sec-t2.tv_sec)+t3.tv_usec-t2.tv_usec;
        timeuse/=1000000;
	printf("%f\n", timeuse);
	
	return 0;

}


