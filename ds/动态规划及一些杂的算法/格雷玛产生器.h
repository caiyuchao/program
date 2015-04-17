//  格雷码产生器 
int DG( int x ){
    return x^( x>>1 );
}
int GD( int x ){
    int y = x;
    while( x>>=1 )y ^= x;
    return y;
}       
int solve( int n, int m, int *a ){         
    // 产生一个长为 n ，以 m 没开头 的格雷 码 
    // 例如 n = 4, m = 2 时 a[] = { 2, 0, 1, 3 };
    // a[] 保存产生的 格雷码 
    int  i, top;
    top = 0;
    a[top++] = m;
    m = GD( m );
    for( i = 1;i < n; i++ ){
        m++;
        if ( m == n )m = 0;
        a[top++] = DG( m );
    }
    return top; // top == n  
}
