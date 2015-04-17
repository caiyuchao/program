// 二维树状数组
#include<stdio.h>
#include<string.h>
int n, c[1025][1025];
int low( int k )  {
    return (k&(k^(k-1)));
}
void update( int x, int y, int a ) // 更新 
{
    int ix;
    x = x+1;
    y = y+1;
    while( y <= n ){
        ix = x;
        while(ix<=n){
            c[ix-1][y-1] = c[ix-1][y-1]+a;
            ix = ix+low(ix);
        }
        y = y+low(y);
    }
}

int sum( int x1, int y1, int x2, int y2 )  //求[x1,y1] - [x2,y2] 的和 
{
    int res;
    int ix1, ix2, iy1, iy2;
    res = 0;
    iy2 = y2+1;
    while( iy2 > y1 ){
        ix2 = x2 + 1;
        while( ix2 > x1 ){
            res += c[ix2-1][iy2-1];
            ix2 -= low(ix2);
        }
        ix1 = x1;
        while( ix1 > ix2 ){
            res -= c[ix1-1][iy2-1];
            ix1 -= low(ix1);
        }
        iy2 = iy2 - low(iy2);
    }
    iy1 = y1;
    while( iy1 > iy2 ){
        ix2 = x2+1;
        while( ix2 > x1 ){
            res -= c[ix2-1][iy1-1];
            ix2 -= low(ix2);
        }
        ix1 = x1;
        while( ix1 > ix2 ){
            res += c[ix1-1][iy1-1];
           ix1 -= low(ix1);
        }
        iy1 = iy1 - low(iy1);
    }
    return res;
}

int main(){
    int now, x, y, a, l, b, r, t;
    while( scanf( "%d", &now ) && now != 3 ){
        if( now == 0 ){
            scanf( "%d", &n );
            for( int i = 0; i <= n; i++ ){
                for( int j = 0; j <= n; j++ )c[i][j] = 0;
            }
        }
        else if( now == 1 ){
            scanf( "%d %d %d", &x, &y, &a );
            update(x,y,a);
        }
        else if( now == 2 )
        {
            scanf( "%d %d %d %d", &l, &b, &r, &t );
            printf( "%d\n", sum(l,b,r,t) );
        } 
    }
    return 0;
}
