#include<stdio.h>
#include<string.h>
int c[1025][1025];
int low( int k )
{
    return (k&(k^(k-1)));
}
int n, m;
void update( int x, int y )
{
	int i,j,a,b;
	for(i=x;i<=n;i+=low(i))
		for(j=y;j<=n;j+=low(j)) 
        { 		
			c[i][j] += 1;
		}
}
int sum(int x,int y) 
{
	int i,j,a,b,t=0;
	for(i=x;i>0;i-=low(i))
		for(j=y;j>0;j-=low(j)) 
        {
			 t += c[i][j];
		}
	return t%2;
}
int main()
{
    int now, x1, y1, x2, y2, a, l, b, r, t, i, j;
    int kcase;
    char s[10];
    scanf( "%d", &kcase );
    int flag = 0;
    while( kcase-- )
    {
        if( flag )printf( "\n" );
        flag = 1;
        scanf( "%d %d", &n, &m );
        for( i = 0; i <= n; i++ )
        {
            for( j = 0; j <= n; j++ )c[i][j] = 0;
        }
        for( i = 0; i < m; i++ )
        {
            scanf( "%s", s );
            if( s[0] ==  'C' )
            {
                scanf( "%d %d %d %d", &x1, &y1, &x2, &y2 );
                update( x1, y1 );
                update( x1, y2+1 );
                update( x2+1, y1 );
                update( x2+1, y2+1 );
            }
            else
            {
                scanf( "%d %d", &l, &b );
                printf( "%d\n", sum(l,b)%2 );
            } 
        }
    }
    return 0;
}
