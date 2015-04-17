#include<stdio.h>
int getmax( int *a, int n ){
    int i, j, k, r, t;
    int m[101][101];
    for( i = 1; i <= n; i++ )m[i][i] = 0;
    for( r = 2; r <= n; r++ ){
        for( i = 1; i <= n-r+1; i++ ){
            j = i+r-1;
            m[i][j] = m[i+1][j] + a[i-1]*a[i]*a[j];
            for( k = i+1; k < j; k++ ){
                t = m[i][k] + m[k+1][j] + a[i-1]*a[k]*a[j];
                if( t < m[i][j] )m[i][j] =  t;
            }
        }
    }
    return m[1][n-1];
}
int main(){
    int i, n, a[101];
    while( scanf( "%d", &n ) && n ){
        for( i = 0; i < n; i++ ){
            scanf( "%d", &a[i] );
        }
        printf( "%d\n", getmax( a, n ) );
    }
    return 0;
}
