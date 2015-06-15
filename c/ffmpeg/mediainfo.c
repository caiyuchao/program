#include <stdio.h> 
#include <string.h> 

int get_probe( const char* filename, char* info);
int ffmpeg(char *cmd);

int main(){
    int ret,argc;
    char info[10240];
    int width=120;
    int height=90;
    char *inputfile = "/root/1.3gp";
    char *outputfile = "/root/1.jpg";
	int  starttime = 10;
	char cmd[1024];


	sprintf(cmd,"ffmpeg -ss %d -i %s -y -vframes 1 -s %d*%d %s", starttime, inputfile, width, height, outputfile);

    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    
    ret = ffprobe(inputfile,info);
    if (ret) return ret; 
    printf("info is [%s]\n",info);
	
	ret = ffmpeg(cmd);
    if (ret) return ret; 

	return 0;
    
}
