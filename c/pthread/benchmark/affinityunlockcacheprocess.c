#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#define __USE_GNU
#include <pthread.h>
#include <sched.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
	
//static inline _syscall0(pid_t,gettid)
#define gettid() syscall(__NR_gettid)

#define ORANGE_MAX_VALUE      1000000
#define APPLE_MAX_VALUE       100000000
#define MSECOND               1000000
	
struct apple
{
	unsigned long long a;
	char c[128];
	unsigned long long b;
};
	
struct orange
{
	int a[ORANGE_MAX_VALUE];
	int b[ORANGE_MAX_VALUE];
};

	
int cpu_nums = 0;
	
inline int set_cpu(int i)
{
	cpu_set_t mask;
		
	CPU_ZERO(&mask);
		
	if(2 <= cpu_nums)
	{
		CPU_SET(i,&mask);
			
		if(-1 == sched_setaffinity(gettid(),sizeof(&mask),&mask))
		{
                        printf("set affinity error\r\n");
			return -1;
		}
	}
	return 0;
}
	
	
void* addx(void* x)
{
	unsigned long long sum=0;
	struct timeval tpstart,tpend; 
	float  timeuse; 
	
	if(-1 == set_cpu(1))
	{
		return NULL;
	}
		
	gettimeofday(&tpstart,NULL); 
		
	for(sum=0;sum<APPLE_MAX_VALUE;sum++)
	{
		((struct apple *)x)->a += sum;
	}
		
	gettimeofday(&tpend,NULL); 
		
	timeuse=MSECOND*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=MSECOND; 
	printf("addx thread:%x,Used Time:%f\n",/*pthread_self()*/gettid(),timeuse);
		
	return NULL;
}
	
void* addy(void* y)
{
	unsigned long long sum=0;
	struct timeval tpstart,tpend; 
	float  timeuse; 
	
	if(-1 == set_cpu(0))
	{
		return NULL;
	}
		
	gettimeofday(&tpstart,NULL); 
		
	for(sum=0;sum<APPLE_MAX_VALUE;sum++)
	{
		((struct apple *)y)->b += sum;
	}
		
	gettimeofday(&tpend,NULL); 
		
	timeuse=MSECOND*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=MSECOND; 
	printf("addy thread:%x,Used Time:%f\n",/*pthread_self()*/gettid(),timeuse);
		
	return NULL;
}
	
	
	
int main (int argc, const char * argv[]) {
		// insert code here...
	struct apple test;
	struct orange test1={{0},{0}};
	pthread_t ThreadA,ThreadB;
		
	unsigned long long sum=0,index=0;
	struct timeval tpstart,tpend; 
	float  timeuse; 
		
	test.a= 0;
	test.b= 0;
	
	cpu_nums = sysconf(_SC_NPROCESSORS_CONF);
	
	if(-1 == set_cpu(0))
	{
		return 1;
	}
		
	gettimeofday(&tpstart,NULL); 
		
	pthread_create(&ThreadA,NULL,addx,&test);
	pthread_create(&ThreadB,NULL,addy,&test);
		
	for(index=0;index<ORANGE_MAX_VALUE;index++)
	{
		sum+=test1.a[index]+test1.b[index];
	}
		
	pthread_join(ThreadA,NULL);
	pthread_join(ThreadB,NULL);
		
	gettimeofday(&tpend,NULL); 
		
	timeuse=MSECOND*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec; 
	timeuse/=MSECOND; 
	printf("main thread:%x,Used Time:%f\n",/*pthread_self()*/gettid(),timeuse);	
		
		
	printf("a = %llu,b = %llu,sum=%llu\n",test.a,test.b,sum);
		
	return 0;
}
	
#ifdef __cplusplus
}
#endif


