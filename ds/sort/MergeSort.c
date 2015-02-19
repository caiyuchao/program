#include<stdio.h>
#include <stdlib.h>
#define MAX 1024
int SegmentMerge(int r[],int low,int high,int len);

void MergeSort(int r[],int n)
{
    int low,high,len;
    len=1;//先将序列每元素看成是长度为1的子序列只要子序列长度小于n
    while(len<n)
    {
        low=0;//low开始计算，至少存在2个子序列没有合并时继续合并
        while(len+low<n)
        {
            high=low+len*2-1;
            if(high>=n)high=n-1;
            if(!SegmentMerge(r,low,high,len))
                return;
            low=high+1;
        }
        len*=2;
    }
}
int SegmentMerge(int r[],int low,int high,int len)
{
    int *r1,*r2;
    int size1,size2;
    int i,j,k;
    size1=len;
    size2=high-low+1-len;
    r1=(int*)malloc(size1*sizeof(int));
    r2=(int*)malloc(size2*sizeof(int));
    if(r1==NULL||r2==NULL)
        return 0;
    //将r[low...low+size1-1]与r[low+size1...low+size1+size2-1]复制到r1、r2
    for(i=0;i<size1;i++)
    {
        r1[i]=r[low+i];
    }
    for(i=0;i<size2;i++)
    {
        r2[i]=r[low+size1+i];
    }
    i=0;
    j=0;
    k=low;
    while(i<size1&&j<size2)
    {
        //合并r[low...high]
        if(r1[i]<=r2[j])
            r[k++]=r1[i++];
        else 
            r[k++]=r2[j++];
    }
    while(i<size1)r[k++]=r1[i++];
    while(j<size2)r[k++]=r2[j++];
    free(r1);
    free(r2);
    return 1;
}

int main(int argc, char *argv[])
{                       
    int n,i,a[MAX];     

    scanf("%d",&n);

    for(i=0;i<n;i++)
        scanf("%d",&a[i]);

    MergeSort(a,n);

    printf("%s:\n", argv[0]);
    for(i=0;i<n;i++)
        printf("\t%d",a[i]);
    printf("\n");  

} 
