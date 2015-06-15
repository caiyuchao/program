#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>


#define MSGSZ	 128


/*
 * Declare the message structure.
 */

typedef struct msgbuf {
	long	mtype;
	char	mtext[MSGSZ];
} message_buf;

static FILE *fp;

static void done(int sig){
	fputs("DONE...\n",fp);
	fclose(fp);
	exit (1);

}


int main()
{
	int msqid;
	key_t key;
	message_buf  rbuf;
	struct msqid_ds msg_info;
	int pid;
	char buff[1024];


	signal( SIGQUIT, SIG_IGN ) ;
	signal( SIGCHLD, SIG_IGN ) ;
	signal( SIGINT,  SIG_IGN ) ;
	signal( SIGHUP,  SIG_IGN ) ;
	signal( SIGTERM, done );
	if ( (pid = fork()) < 0){
		printf("fork error\n");
		return 1;
	}
	fp = fopen("demo.log","a+");
	if(pid) return 0;


	key = 1234;

	if ((msqid = msgget(key, 0666)) < 0) {
		printf("msgget error \n");
		return 1;
	}

	
	while(1){
		if (msgrcv(msqid, &rbuf, MSGSZ, 1, 0) < 0) {
	   		printf("msgrcv error \n");
			return 1;
		}
		msgctl(msqid, IPC_STAT, &msg_info);

		sprintf(buff, "%s %s",  ctime(&msg_info.msg_rtime), rbuf.mtext);
		fputs(buff, fp);
		fflush(fp);
		if(strcmp(rbuf.mtext,"EOF\n") == 0)
			break;
	}
	fputs(" colse .... \n", fp);
	fclose(fp);
	return 0;

}


