/*
对有根树T进行DFS，将遍历到的结点按照顺序记下，
我们将得到一个长度为2N – 1的序列，称之为T的欧拉序列F
每个结点都在欧拉序列中出现，我们记录结点u在欧拉序列中
第一次出现的位置为pos(u)
根据DFS的性质，对于两结点u、v，从pos(u)遍历到pos(v)的过程中经过LCA(u, v)
有且仅有一次，且深度是深度序列B[pos(u)…pos(v)]中最小的
即LCA(T, u, v) = RMQ(B, pos(u), pos(v))，并且问题规模仍然是O(N)的
*/

#include<stdio.h>
#include<string.h>
#include<vector>
#include<iostream>
#include<cmath>
using namespace std;
vector<int>v[10005];
int n;
int pos[10005];
bool flag[10005];
int B[10005*2];
int F[10005*2];
int f[10005*2][18];
int cnt;

void dfs( int u, int dep ){  //得到B， F， POS   
    int i;
    for( i = 0; i < v[u].size(); i++ ){
        cnt++;
        if( pos[u] == -1 )pos[u] = cnt;
        B[cnt] = dep;
        F[cnt] = u;
        dfs(v[u][i],dep+1);
    }
    cnt++;
    if( pos[u] == -1 )pos[u] = cnt;
    F[cnt] = u;
    B[cnt] = dep;
}

void Make_RMQ(){
    int i, j, k;
    for( i = 1; i <= cnt; i++ )f[i][0] = i;
    for( i = 1; (1<<i) <= cnt+1; i++ ){
        for( j = 1;j+(1<<i)-1 <= cnt;j++ ){
            f[j][i] = f[j][i-1];
            if( B[f[j][i]] > B[f[j+(1<<(i-1))][i-1]] ){
                f[j][i] = f[j+(1<<(i-1))][i-1];
            }
        }
    }
    
}

int RMQ_FIND(int l, int r){
	int k = (int)trunc(log((double)(r-l+1))/log(2.0));
	return ( B[f[l][k]] < B[f[r-(1<<k)+1][k]] ) ? f[l][k]: f[r-(1<<k)+1][k];
}

int main(){
    int i, n, kcase, x, y;
    scanf( "%d", &kcase );
    while( kcase-- ){
        scanf( "%d", &n );
        for( i = 1; i <= n; i++ ){
            pos[i] = -1;
            v[i].clear();
            flag[i] = false;
        }
        for( i = 1; i < n; i++ ){
            scanf( "%d %d", &x, &y );
            v[x].push_back(y);
            flag[y] = true;
        }
        for( i = 1; flag[i]; i++ );  //找根接点 
        cnt = 0;
        dfs( i, 0 );  
        
        Make_RMQ(); //建立RMQ 
        
        
        scanf( "%d %d", &x, &y ); // 查询，O（1） 
        x = pos[x];
        y = pos[y];
        if( x > y ){ int t = x; x = y; y = t; }
        printf( "%d\n", F[RMQ_FIND(x,y)] );
        
        /*
        int ans = x;   //如果只查询一次的话就直接遍历，不需要建立RMQ了 
        for( i = x+1; i <= y; i++ )
        {
            if( B[i] < B[ans] )ans = i;
        }
        printf( "%d\n", F[ans] );
        */
        
    }
    return 0;
    
}

