#include <iostream>
using namespace std;
const int maxn = 100000*2;
int  c[maxn], h[maxn], smem[3][maxn];
int *SA, *nSA, *Rank, *nRank;
int MAX, n, K;
char num[maxn];
int len2, len1;
void suffix_array()
{
	int i, j, k;
	SA=smem[0], nSA=smem[1], Rank=smem[2];
	memset( c, 0, sizeof(c) );
	for( i = 0; i < n; i++ ) c[num[i]+1]++;
	for( i = 1; i < MAX; i++ ) c[i]+=c[i-1];
	for( i = 0; i < n; i++ )  SA[--c[num[i]+1]]=i;
	for( Rank[SA[0]] = 0,i = 1; i < n; i++ ){
		Rank[SA[i]] = Rank[SA[i-1]];
		if( num[SA[i-1]] != num[SA[i]] )
			Rank[SA[i]]++;
	}
	for( k = 1; k < n && Rank[ SA[n-1] ] < n-1; k *= 2 ){
		for( i = 0; i < n;i++ )c[Rank[SA[i]]] = i+1;
		for( i = n-1; i >= 0;i-- ){
			if( SA[i] >= k ){
				nSA[ --c[Rank[SA[i]-k]] ] = SA[i]-k;
            }
        }
		for( i = n-k; i < n; i++ ){
			nSA[ --c[Rank[i]] ] = i;
        }
		nRank = SA, SA = nSA;
		for( nRank[SA[0]] = 0, i = 1; i < n; i++ ){
			nRank[SA[i]] = nRank[SA[i-1]];
			if( Rank[SA[i]] != Rank[SA[i-1]] || Rank[SA[i]+k] != Rank[SA[i-1]+k] ){
				nRank[SA[i]]++;
            }
		}
		nSA = Rank; 
        Rank = nRank;
	}
}
int get_lcp(){
	int i, j, k, ret, t;
	for( i = 0, k = 0; i < n; i++ ){
		if( Rank[i] == n-1 )
			h[Rank[i]] = k = 0;
		else{
			if( k  > 0 ){
				k--;
            }
			j = SA[Rank[i]+1];
			while( num[i+k] == num[j+k] ){
				k++;
            }
			h[Rank[i]] = k;
		}
	}
	ret = 0;
	for( i = 0; i < n-1; i++ )  // 按题目要求求解  该处是求两个串的最长公共子串 
	{
		if( ret < h[i] && (SA[i]<len1) != (SA[i+1]<len1) )
		{
			ret = h[i];
        }
    }
	return ret;
}
int main()
{
	gets( num );
	len1=strlen( num );
	gets( num + len1 );
	len2=strlen( num + len1 );
	n=len1 + len2;
	num[n++] = 1;
	MAX = 200;  //设置串中可能出现的最大值 
	suffix_array();
	printf( "%d\n",get_lcp() );
	return 0;
}

