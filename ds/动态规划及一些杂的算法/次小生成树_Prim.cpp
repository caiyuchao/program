#include<stdio.h>
#include<string.h>

const int maxn = 1005;
const int INF = 0x7fffffff;
class SecondMST
{
public:
    bool f[maxn];
    bool h[maxn][maxn];
    int g[maxn], mt[maxn];
    int cost[maxn][maxn], a[maxn][maxn];
    int i, j, k, n, m, w;
    int o, min;
    bool y;
    bool ok;
    bool Init()
    {
        if( scanf( "%d %d", &n, &m ) == EOF )return false;
        ok = true;
        for( i = 1; i <= n; i++ )
        {
            for( j = 1; j <= n; j++ )cost[i][j] = INF;
        }
        for( i = 1; i <= m; i++ )
        {
            scanf( "%d %d %d", &j, &k, &w );
            cost[k][j] = cost[j][k] = w;
        }
        return true;
    }
    
    void Get_MST()
    {
        for( i = 1; i <= n; i++ )
        {
            mt[i] = cost[1][i];
            g[i] = 1;
        }
        memset( f, 0, sizeof( f ) );
        f[1] = true;
        o = 0;
        for( i = 1; i < n; i++ )
        {
            min = INF;
            for( j = 1; j <= n; j++ )
            {
                if( !f[j] && mt[j] < min )
                {
                    min = mt[j];
                    k = j;
                }
            }
            //if( min == INF ){ return; ok = false; }
            f[k] = true;
            o += min;
            for( j = 1; j <= n; j++ )
            {
                if( !f[j] && cost[j][k] < mt[j] )
                {
                    mt[j] = cost[j][k];
                    g[j] = k;
                }
            }
        }
    }
    void go(int l, int i, int o )
    {
        int j;
        for( j = 1; j <= n; j++ ){
            if( h[i][j] && !f[j] )
            {
                f[j] = true;
                if( cost[i][j] > o )
                {
                    a[l][j] = cost[i][j];
                    a[j][l] = a[l][j];
                    go( l, j, cost[i][j] );
                }
                else
                {
                    a[l][j] = o;
                    a[j][l] = o;
                    go(l,j,o);
                }
            }
        }
    }
    
    void Get_secondMST()
    {
        //if( ok == false )return;
        for( i = 1; i <= n; i++ )
        {
            h[i][g[i]] = true;
            h[g[i]][i] = true;
        }
        
        for( i = 1; i <= n; i++ )
        {
            for( j = 1; j <= n; j++ )f[j] = false;
            f[i] = true;
            go(i,i,0);
        }
        min = INF;
        for( i = 1; i < n; i++ )
        {
            for( j = i+1; j <= n; j++ )
            {
                if( cost[i][j] < INF && !h[i][j] && o+cost[i][j]-a[i][j] < min )
                {
                    min = o+cost[i][j]-a[i][j];
                }
            }
        }
    }
    void display()
    {
        //if( ok == false )printf( "Cost: %d\nCost %d\n", -1, -1 );
        //else 
        //{
            //printf( "Cost: %d\n", o );
            if( min == INF )printf( "Cost: -1\n" );
            else printf( "Cost: %d\n", min );
        //}
    }
};
        
SecondMST T;

int main()
{
    while( T.Init() )
    {
        T.Get_MST();
        T.Get_secondMST();
        T.display();
    }
    return 0;
}
