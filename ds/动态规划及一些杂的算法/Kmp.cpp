#include<stdio.h>
#include<string.h>
char s[1000001], t[1000001];
int next[1000001],lens, lent;

void getnext( char *t, int *next ){
	int j = 1, k = 0, len;
	next[0] = -1;
	next[1] = 0;
	while( j < lent ){
		if( t[j] == t[k] ){
			next[j+1] = k+1;
			j++;
			k++;
		} else if( k == 0 ){
			next[j+1] = 0;
			j++;
		} else{
			k = next[k];
		}
	}
	for(j=0; j<lent; j++){
		printf("%c %d\n", t[j], next[j]);
	}
}

int KMPindex( char *s, char *t, int *next ){
	int i, j, cnt;
	cnt = 0;
	i = 0;
	j = 0;
	while( i < lens ){
		if( s[i] == t[j] ){
			i++;
			j++;
		}else if( j == 0 ){
			i++;
		} else {
			j = next[j];
		}
		if( j == lent ){
			j = next[j];
			cnt++;
		}
	}
	return cnt;
}

int main(){
	int p;
	scanf( "%d", &p );
	while( p-- ){
		scanf( "%s %s", s, t );
		lens = strlen( s );
		lent = strlen( t );
		getnext( t, next );
		printf( "%d\n", KMPindex( s, t, next ) );
	}
	return 0;
}
