#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int buf[200005];
int *p[200005];
int n, best, pans;
int cmp( const void *a, const void *b )
{
    int *c = *(int **)a;
    int *d = *(int **)b;
    int i;
    for( i = 0; c[i] != -1 && d[i] != -1; i++ )
    {
        if( c[i] != d[i] )return c[i]-d[i];
    }
    return c[i]-d[i];
}
int getpve(int *a, int *b )
{
    int i;
    for( i = 0; a[i] != -1 && a[i] == b[i]; i++ );
    return i;
}
int main()
{
    int i, j, c, K;
    while( scanf( "%d %d", &n, &K ) != EOF )
    {
        for( i = 0; i < n; i++ )
        {
            scanf( "%d", buf+i);
            p[i] = buf+i;
        }
        buf[n] = -1;
        qsort( p, n, sizeof( p[0] ), cmp );
        best = 0;
        for( i = K-1; i < n; i++ )
        {
            pans = getpve(p[i],p[i-K+1]);
            if( pans > best )best = pans;
        }
        printf( "%d\n", best );
    }
    return 0;
}
