
const int maxn = 100000;
class GetLIS()
{
public:
    int c[maxn];  // ±£¥Êans 
    int q[maxn];  // ≥ı º–Ú¡– 
    int Bisearch(int pre, int post, int v) 
    {
        int mid;
        while ( pre <= post ) 
        {
            mid = ( pre + post ) >> 1;
            if ( c[mid] < v ) pre = mid+1;
            else post = mid-1;
        }
        return pre;
    }
    void LIS()
    {
        int i, t;
        int top = 0;
        for( i = 1; i <= n; i++ )
        {
            t = Bisearch( 0, top, q[i] );
            c[t] = q[i]; 
            if ( t > top )top++;
        }
        printf( "%d\n", top );
    }
};
