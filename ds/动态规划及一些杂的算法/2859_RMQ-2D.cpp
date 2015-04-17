//RMQ-2D example 2859


#include<stdio.h>
const int MAXN=301;
const int MAXM=301;
const int MAXF=9;
const int MAXE=9;
const int INF=0x7FFFFFFF;
inline short max(short a,short b){return a<b?a:b;}
inline short max(short a,short b,short c,short d){return a<?=b<?=c<?=d;}
int a[MAXN][MAXM], n, m;
class
{
    short dp[MAXN][MAXF+1][MAXM][MAXE+1];
    //dp[i][f][j][e]表示从以(i,j)为左上角元素大小为2^f * 2^e的矩阵中的最大(小)值
public:
	void init(){
		for(int i=0;i<n;i++){
			for(int j=0;j<m;j++){
				dp[i][0][j][0]=a[i][j];
            }
        }		
		for(int j=0;j<m;j++){
		    for(int f=1,s=1;s<n;s=(1<<f++)){
			    for(int i=0;i+s<n;i++){
				    dp[i][f][j][0]=max(dp[i][f-1][j][0],dp[i+s][f-1][j][0]);
                }
            }
        }
	    for(int i=0;i<n;i++){
		    for(int e=1,r=1;r<m;r=(1<<e++)){
			    for(int j=0;j+r<m;j++){
				    dp[i][0][j][e]=max(dp[i][0][j][e-1],dp[i][0][j+r][e-1]);
                }
            }
        }
		for(int f=1,s=1;s<n;s=(1<<f++)){
			for(int e=1,r=1;r<m;r=(1<<e++)){
			    for(int i=0;i+s<n;i++){
			    	for(int j=0;j+r<m;j++){
				        dp[i][f][j][e]=max(dp[i][f-1][j][e-1],
				                       dp[i+s][f-1][j][e-1],
				                       dp[i][f-1][j+r][e-1],
				                       dp[i+s][f-1][j+r][e-1]);
                    }
                }
            }
        }
	}
	//返回矩阵(x1,y1)--(x2,y2)中的最值
	short query(int x1,int y1,int x2,int y2){
	    if(x1>x2 || y1>y2)return -INF;
		int dx=x2-x1+1;
		int dy=y2-y1+1;
		int f,e;
		for(f=0;(1<<f)<=dx;f++);
		for(e=0;(1<<e)<=dy;e++);
		f--;e--;
		return max(dp[x1][f][y1][e],
		dp[x2-(1<<f)+1][f][y1][e],
		dp[x1][f][y2-(1<<e)+1][e],
		dp[x2-(1<<f)+1][f][y2-(1<<e)+1][e]);
	}
}W;


int main()
{
    int kcase, i, j, T, x1, y1, x2, y2;
    
    scanf( "%d", &kcase );
    
    while( kcase-- )
    {
        scanf( "%d", &n );
        m = n;
        for( i = 0; i < n; i++ )
        {
            for( j = 0; j < n; j++ )
            {
                scanf( "%d", &a[i][j] );
            }
        }
        W.init();
        scanf( "%d", &T );
        for( i = 0; i < T; i++ )
        {
            scanf( "%d %d %d %d", &x1, &y1, &x2, &y2 );
            printf( "%d\n", W.query(x1-1,y1-1,x2-1,y2-1) );
        }
    }
    return 0;
}



