#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
main()
{
	int pipe_fd[2];
	pid_t pid;
	char r_buf[4];
	char* w_buf;
	int writenum;
	int cmd;
	
	memset(r_buf,0,sizeof(r_buf));
	if(pipe(pipe_fd)<0)
	{
		printf("pipe create error\n");
		return -1;
	}
	
	if((pid=fork())==0)
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		sleep(10);	
		return 0;
	}
	else if(pid>0)
	{
	sleep(1);  //�ȴ��ӽ�����ɹرն��˵Ĳ���
	close(pipe_fd[0]);//write
	w_buf="111";
	if((writenum=write(pipe_fd[1],w_buf,4))==-1)
		printf("write to pipe error\n");
	else	
		printf("the bytes write to pipe is %d \n", writenum);
	
	close(pipe_fd[1]);
	}	
}
