#include<string.h>
#include<stdio.h>
const int maxn = 55;
typedef struct{
     int dir[maxn];
     int size;
}Node;
class MaxClique{
public:
    int n, i, j;
    int c[maxn];
    int max;
    int map[maxn][maxn];
    Node v[maxn];
    bool Init()  //≥ı ºªØ 
    {
        scanf( "%d", &n );
        if( n == 0 )return false;
        for( i = 0; i < n; i++ ){
            v[i].size = 0;
            for( j = 0; j < n; j++ ){
                scanf( "%d", &map[i][j] );
                v[i].dir[j] = map[i][j];
                if( map[i][j] )v[i].size++;
            }
        }
        return true;
    }
    Node set( Node a, Node b ){
        Node t;
        t.size = 0;
        for( int i = 0; i < n; i++ ){
            t.dir[i] = ( a.dir[i] && b.dir[i] );
            if( t.dir[i] )t.size++;
        }
        return t;
    }
    void Get_MaxClique( Node q, int cnt ){
        int i;
        Node t;
        if( q.size == 0 ){
            if( cnt > max ){
                max = cnt;
            }
            return;
        }
        i = 0;
        while( q.size ){
            if( q.size+cnt < max )return;
            for( ; !q.dir[i]; i++ );
            q.dir[i] = 0;
            q.size--;
            if( cnt+c[i] < max )return;
            t = set(q,v[i]);
            Get_MaxClique( t, cnt+1 );
        } 
    }
    void work()  // get ans
    {
        Node p, q;
        max = 0;
        p.size = 0;
        for( i = 0; i < n; i++ )p.dir[i] = 0;
        for( i = n-1; i >= 0; i-- ){
            p.size++;
            p.dir[i] = 1;
            q = set( p, v[i] );
            Get_MaxClique( q, 1 );
            c[i] = max;
        }
        printf( "%d\n", max ); 
    }
};
MaxClique T;
int main(){
    while( T.Init() ){
        T.work();
    }
    return 0;
}

