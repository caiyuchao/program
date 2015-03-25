//欧拉数(递推法)
#include<stdio.h>
const int MAX = 1000000;
int prime[MAX+1] = {0};
__int64 e[MAX+1] = {0, 1};
void makeprime(){
	for(int i = 2; i*i <= MAX; i++){
		for(int j = i*i; j <= MAX; j+=i) if(!prime[j])
			prime[j] = i;
	}
}
void init(){
	makeprime();
	for(int i = 2; i <= MAX; i++){
		if(!prime[i])
			e[i] = i-1;
		else{
			int k = i/prime[i];
			if(k%prime[i])
				e[i] = (prime[i]-1)*e[k];
			else
				e[i] = prime[i]*e[k];
		}
	}
	for(int j = 3; j <= MAX; j++)
		e[j] += e[j-1];
}
int main(){
	init();
	int n;
	while(scanf("%d", &n), n)
		printf("%I64d\n", e[n]);
	return 0;
}
