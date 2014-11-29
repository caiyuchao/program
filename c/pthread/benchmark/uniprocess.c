#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
	
#define ORANGE_MAX_VALUE      1000000
#define APPLE_MAX_VALUE       100000000
#define MSECOND               1000000

struct apple
{
    unsigned long long a;
	unsigned long long b;
};

struct orange
{
	int a[ORANGE_MAX_VALUE];
	int b[ORANGE_MAX_VALUE];
	
};


int main (int argc, const char * argv[]) {
    // insert code here...
    struct apple test;
	struct orange test1={{0},{0}};
	
	unsigned long long sum=0,index=0;
	struct timeval tpstart,tpend;
	float  timeuse; 
	
    test.a= 0;
	test.b= 0;
	
	/*get start time*/
	gettimeofday(&tpstart,NULL); 
	
	for(sum=0;sum<APPLE_MAX_VALUE;sum++)
	{
		test.a += sum;
		test.b += sum;
	}
	
        sum=0;

	for(index=0;index<ORANGE_MAX_VALUE;index++)
	{
		sum+=test1.a[index]+test1.b[index];
	}

	/*get start time*/
	gettimeofday(&tpend,NULL); 
	
	/*calculate time*/
	timeuse=MSECOND*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=MSECOND; 
	printf("main thread:%x,Used Time:%f\n",pthread_self(),timeuse);	
	
	printf("a = %llu,b = %llu,sum=%llu\n",test.a,test.b,sum);
	
    return 0;
}
	

#ifdef __cplusplus
}
#endif
