/*
���и���T����DFS�����������Ľ�㰴��˳����£�
���ǽ��õ�һ������Ϊ2N �C 1�����У���֮ΪT��ŷ������F
ÿ����㶼��ŷ�������г��֣����Ǽ�¼���u��ŷ��������
��һ�γ��ֵ�λ��Ϊpos(u)
����DFS�����ʣ����������u��v����pos(u)������pos(v)�Ĺ����о���LCA(u, v)
���ҽ���һ�Σ���������������B[pos(u)��pos(v)]����С��
��LCA(T, u, v) = RMQ(B, pos(u), pos(v))�����������ģ��Ȼ��O(N)��
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

void dfs( int u, int dep ){  //�õ�B�� F�� POS   
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
        for( i = 1; flag[i]; i++ );  //�Ҹ��ӵ� 
        cnt = 0;
        dfs( i, 0 );  
        
        Make_RMQ(); //����RMQ 
        
        
        scanf( "%d %d", &x, &y ); // ��ѯ��O��1�� 
        x = pos[x];
        y = pos[y];
        if( x > y ){ int t = x; x = y; y = t; }
        printf( "%d\n", F[RMQ_FIND(x,y)] );
        
        /*
        int ans = x;   //���ֻ��ѯһ�εĻ���ֱ�ӱ���������Ҫ����RMQ�� 
        for( i = x+1; i <= y; i++ )
        {
            if( B[i] < B[ans] )ans = i;
        }
        printf( "%d\n", F[ans] );
        */
        
    }
    return 0;
    
}

