#include <iostream>
using namespace std;
const int maxn = 20005*2;
int h[maxn], smem[3][maxn];
int *SA, *nSA, *Rank, *nRank;
int c[1000005];
int MAX, n, K;
int num[maxn];
int len2, len1;

void suffix_array()
{
	int i, j, k;

	SA=smem[0], nSA=smem[1], Rank=smem[2];
	for( i = 0; i <= MAX; i++ )c[i] = 0;
	for( i = 0; i < n; i++ ) c[num[i]]++;
	for( i = 1; i <= MAX; i++ ) c[i]+=c[i-1];
	for( i = 0; i < n; i++ )  SA[--c[num[i]]]=i;
	
	for( Rank[SA[0]] = 0,i = 1; i < n; i++ )
	{
		Rank[SA[i]] = Rank[SA[i-1]];
		if( num[SA[i-1]] != num[SA[i]] )
			Rank[SA[i]]++;
	}
	for( k = 1; k < n && Rank[ SA[n-1] ] < n-1; k *= 2 )
	{
		for( i = 0; i < n;i++ )c[Rank[SA[i]]] = i+1;
		for( i = n-1; i >= 0;i-- )
		{
			if( SA[i] >= k )
			{
				nSA[ --c[Rank[SA[i]-k]] ] = SA[i]-k;
            }
        }
		for( i = n-k; i < n; i++ )
		{
			nSA[ --c[Rank[i]] ] = i;
        }
		nRank = SA, SA = nSA;
		for( nRank[SA[0]] = 0, i = 1; i < n; i++ )
		{
			nRank[SA[i]] = nRank[SA[i-1]];
			if( Rank[SA[i]] != Rank[SA[i-1]] || Rank[SA[i]+k] != Rank[SA[i-1]+k] )
			{
				nRank[SA[i]]++;
            }
		}
		nSA = Rank; 
        Rank = nRank;
	}
}

int get( int i, int j )
{
    int k = 0;
    while( num[i+k] == num[j+k] )
    {
        k++;
    }
    return k;
}

int get_lcp()
{
	int i, j, k, ret, t;
	for( i = 0, k = 0; i < n; i++ )
	{
		if( Rank[i] == n-1 )
			h[Rank[i]] = k = 0;
		else
		{
			if( k  > 0 )
			{
				k--;
            }
			j = SA[Rank[i]+1];
			while( num[i+k] == num[j+k] )
			{
				k++;
            }
			h[Rank[i]] = k;
		}
	}
	ret = 0;
    ret = 0;
    for( i = K-1; i < n; i++ )   // 按题目要求求解，随机应变， 该处是求母串中至少出现K次的最长子串 
    {
        int Max = get( SA[i-K+1],SA[i] );
        if( ret < Max )ret = Max;
    }
	return ret;
}



int main()
{
    int i;
	while( scanf( "%d %d", &len1, &K ) != EOF )
	{
        MAX = 0;
        for( i = 0; i < len1; i++ )
        {
            scanf("%d",num+i);
            num[i]++;
            if( num[i] > MAX )MAX = num[i];
        }
        n = len1+1;
        num[len1] = 0;
        suffix_array();
        printf( "%d\n",get_lcp() );
    }
	return 0;
}

