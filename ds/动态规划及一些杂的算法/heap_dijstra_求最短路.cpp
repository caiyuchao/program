
#include<stdio.h>
#include<vector>
#define min( a, b )  ( a<b?a:b )
#define Inf 200000000
using namespace std;

typedef struct node
{
    int x;  // 邻接节点 
    int w;  // 权值 
}DIS;

vector<DIS>v[10001];
DIS T;

int Max[10001];
int space[10001];
int heap[10001];
int heapsize;
int n, m;
int s, t;
int tag;

void GetDown()
{
    int p = 0; 
    int q = 1;
    int key = Max[heap[0]];
    int x=  heap[0];
    
    while( q < heapsize )
    {
        if( q+1 < heapsize && Max[heap[q+1]] < Max[heap[q]] )q++;
        
        if( Max[heap[q]] > key )break;
        heap[p] = heap[q];
        space[heap[p]] = p;
        p = q;
        q = 2*p+1;
    }
    
    heap[p] = x;
    space[x] = p; 
}

void GetUp( int k )
{
    int q = k;
    int p = (k-1)/2;
    int key = Max[heap[k]];
    int x = heap[k];
    while( q )
    {
        if( Max[heap[p]] < key )break;
        heap[q] = heap[p];
        space[heap[q]] = q;
        q = p;
        p = (q-1)/2;
    }
    heap[q] = x;
    space[x] = q;
}

int GetAns( )
{
    int i, j, k, r, d;
    for( i = 0; i < n; i++ )Max[i] = Inf;  // be MAX
    heapsize = 1;
    heap[0] = s;
    space[s] = 0;
    Max[s] = 0;
    vector <DIS> :: iterator Iter;
    while( true )
    {
        if( heapsize == 0 )return -1;
        k = heap[0];
        heap[0] = heap[--heapsize];
        GetDown();
        if( k == t )return Max[k];
        for ( Iter = v[k].begin (); Iter != v[k].end (); Iter ++ )
        {
            r = Max [k] + Iter->w;
            d = Iter->x;
            if ( Max [d] == Inf ) space[d] = heapsize , heap [heapsize ++] = d;
            if (  r < Max [d] ) Max [d] = r , GetUp ( space[d] );
        }
    }  
}

int main()
{
    int i;
    int z, y, x, r;
    int kcase;
    
    scanf( "%d", &kcase );
    while( kcase-- )
    {
        scanf( "%d %d", &n, &m );
        for( i = 0; i < n; i++ )
        {
            v[i].clear();
        }
        for( i = 0; i < m; i++ )
        {
            scanf( "%d %d %d %d", &x, &y, &r );
            x--;
            y--;
            T.x = y;
            T.w = r;
            
            v[x].push_back( T );
            T.x = x;
            v[y].push_back( T );
        }
        s = 0;  // 源 
        t = n-1;  // 终 
        printf( "%d\n", GetAns() );
    }
    return 0;
    
}

