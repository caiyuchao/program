#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>

#define MSGSZ     128


/*
 * Declare the message structure.
 */

typedef struct msgbuf {
         long    mtype;
         char    mtext[MSGSZ];
         } message_buf;

int main()
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    size_t buf_length;
    int i;
    char line[512];

    key = 1234;


    if ((msqid = msgget(key, msgflg )) < 0) {
        printf("msgget error\n");
        return 1;
    }

    sbuf.mtype = 1;
    
    while(1){
	i = 0;
	while( (line[i++] = getchar() ) != '\n') ;
	line[i++] = '\0';
    	strcpy(sbuf.mtext, line);
    	msgsnd(msqid, &sbuf, i, IPC_NOWAIT); 
    }
      
    return 0;
}


