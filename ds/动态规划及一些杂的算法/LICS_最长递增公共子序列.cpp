//最长公共递增子序列O(n^2)模板ZOJ2432--最长公共递增子序列 
//  时间复杂度 O(n^2), 空间复杂度 O(n^2)
/*
    l1为a的大小, l2为b的大小
    结果在ans中
    f记录路径，DP记录长度
    用a对b扫描，逐步最优化
*/
#include<string.h>
#include<stdio.h>
#include<iostream.h>
#define MAXN 500
typedef int elem_t;
int GCIS(int l1, elem_t a[], int l2, elem_t b[], elem_t ans[])
{
    int f[MAXN+1][MAXN+1];
    int DP[MAXN+1];
    int i,j,k,max;
    memset(f,0,sizeof(f));
    memset(DP,0,sizeof(DP));
    for(i = 1; i <= l1; i++)
    {
        k=0;
        memcpy(f[i],f[i-1],sizeof(f[0]));
        for (j=1;j<=l2;j++)
        {
            if (b[j-1]<a[i-1] && DP[j]>DP[k]) k=j;
            if (b[j-1]==a[i-1] && DP[k]+1>DP[j])DP[j]=DP[k]+1,f[i][j]=i*(l2+1)+k;
        }
    }
    for(max=0,i=1;i<=l2;i++)if (DP[i]>DP[max])max=i;
    
    for(i=l1*l2+l1+max,j=DP[max]; j ;i = f[i/(l2+1)][i%(l2+1)],j--)
    {
        ans[j-1]=b[i%(l2+1)-1];
    }
    return DP[max];
}
 
