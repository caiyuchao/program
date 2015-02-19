/***********************************************************
 * You can use all the programs on  www.c-program-example.com
 * for personal and learning purposes. For permissions to use the
 * programs for commercial purposes,
 * contact info@c-program-example.com
 * To find more C programs, do visit www.c-program-example.com
 * and browse!
 * 
 *                                  Happy Coding
 ***********************************************************/

#include<stdio.h>
#define MAX 1024

int n,a[MAX];     

void ptree(int a[], int len){
    int i, level, m;
    for(i=0, level=1, m=1; i<len; i++){
        printf("%d\t", a[i]);
        if(i==m){
            printf("\n");
            level++;
            m = 1 << level;
        }
    }

}

void heapify(int a[],int n)
{
    int k,i,j,item;

    for(k=1;k<n;k++)
    {
        item = a[k];
        i = k;
        j = (i-1)/2;

        while((i>0)&&(item>a[j]))
        {
            a[i] = a[j];
            i = j;
            j = (i-1)/2;
        }
        a[i] = item;
    }
}

void adjust(int a[],int n)
{
    int i,j,item;

    j = 0;
    item = a[j];
    i = 2*j+1;

    while(i<=n-1)
    {
        if(i+1 <= n-1)
            if(a[i] <a[i+1])
                i++;
        if(item<a[i])
        {
            a[j] = a[i];
            j = i;
            i = 2*j+1;
        }
        else
            break;
    }
    a[j] = item;
}

void heapsort(int a[],int n)
{
    int i,t;

    heapify(a,n);

    for(i=n-1;i>0;i--)
    {
        t = a[0];
        a[0] = a[i];
        a[i] = t;
        adjust(a,i);
    }
}


int main(int argc, char *argv[])
{                       
    int i;     

    scanf("%d",&n);

    for(i=0;i<n;i++)
        scanf("%d",&a[i]);

    heapsort(a,n);

    printf("%s:\n", argv[0]);
    for(i=0;i<n;i++)
        printf("\t%d",a[i]);
    printf("\n");  

} 
