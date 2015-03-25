//Source Code-18 RMQ
//Problem: 2452 <http://acm.pku.edu.cn/JudgeOnline/problem?id=2452>

//Memory: 9100K		Time: 1171MS	
//Language: G++		Result: Accepted	
//Source Code 
#include<stdio.h>
#define MAX 100001
int num[MAX],lg[MAX+1],min[MAX][20],max[MAX][20];
int n,ans;
void init(){
    int i,j,k;
    g[1]=0;
    j=2;
    for(i=2;i<=n;i++) 
    {
        lg[i]=lg[i-1];
        if (i==j) lg[i]++,j*=2;
    }
    for(i=0;i<n;i++) min[i][0]=max[i][0]=i;//³õÊ¼»¯
    for(j=1,k=2;k<=n;j++,k*=2) {
        for(i=0;i+k-1<n;i++) {
            min[i][j]=min[i][j-1];
            max[i][j]=max[i][j-1];
            if (num[min[i+k/2][j-1]]<num[min[i][j]])
            min[i][j]=min[i+k/2][j-1];
            if (num[max[i+k/2][j-1]]>num[max[i][j]])
            max[i][j]=max[i+k/2][j-1];
        }
    }
}

void dfs(int a,int b) {
    if (a>=b) return ;
    int k,minind,maxind;
    k=lg[b-a+1];
    minind=min[a][k];
    if(num[min[b-(1<<k)+1][k]]<num[minind]) minind=min[b-(1<<k)+1][k];
    maxind=max[a][k];
    if(num[max[b-(1<<k)+1][k]]>num[maxind]) maxind=max[b-(1<<k)+1][k];
    if(minind<maxind) 
    {
        if(maxind-minind>ans) ans=maxind-minind;
        dfs(a,minind-1);
        dfs(maxind+1,b);
    }
    else {
        dfs(a,maxind);
        dfs(maxind+1,minind-1);
        dfs(minind,b);
    }
}
	
