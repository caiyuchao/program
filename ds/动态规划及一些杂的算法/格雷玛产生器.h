//  ����������� 
int DG( int x ){
    return x^( x>>1 );
}
int GD( int x ){
    int y = x;
    while( x>>=1 )y ^= x;
    return y;
}       
int solve( int n, int m, int *a ){         
    // ����һ����Ϊ n ���� m û��ͷ �ĸ��� �� 
    // ���� n = 4, m = 2 ʱ a[] = { 2, 0, 1, 3 };
    // a[] ��������� ������ 
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
