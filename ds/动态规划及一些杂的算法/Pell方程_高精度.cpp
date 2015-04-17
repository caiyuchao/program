//pell方程(结果高精度)
//结果长度超过len时，输出No solution!

#include<math.h>
#include<stdio.h>
#include<memory.h>
#define len 1000
#define M 10000
struct BigInt{
	int num[len];
	int top;
};
void mul(BigInt &a, int data){
	int i;
	for (i = 0; i < a.top; i++) a.num[i] *= data;
	for (i = 0; i < a.top+2; i++) if (a.num[i] >= M){
		a.num[i+1] += a.num[i]/M;
		a.num[i] %= M;
	}
	for (i = a.top+4; i > 0; i--) if (a.num[i] > 0) break;
	a.top = i+1;
}
void add(BigInt &a, BigInt b){
	int i, t = a.top>b.top?a.top:b.top;
	for (i = 0; i < t; i++)
		a.num[i] += b.num[i];
	for (i = 0 ;i < t; i++)
		if (a.num[i] >= M){
			a.num[i+1] += a.num[i]/M;
			a.num[i] %= M;
		}
	for (i = t+2; i > 0; i--) if (a.num[i] > 0) break;
	a.top = i+1;
}
void copy(BigInt &a, int n){
	int i;
	memset(a.num, 0, sizeof(a.num));
	if (n==0){
		a.top = 1;
		return;
	}
	for (i = 0; n; i++){
		a.num[i] = n%M;
		n /= M;
	}
	a.top = i;
}
void print(BigInt a){
	int i;
	printf("%d", a.num[a.top-1]);
	for (i = a.top-2; i >= 0; i--)
		printf("%04d", a.num[i]);
}
int ff(BigInt a){
	if (a.top > 250) return 0;
	if (a.top <= 249) return 1;
	if (a.num[249] >= 10000) return 0;
	return 1;
}
int main(){
	int N;
    int a, P, Q, SN;
	BigInt x, y, x1, y1, t;
	bool succ;
    while ( scanf("%d", &N)!=EOF){
		int f = -1;
		SN = (int)sqrt(N);
		if(SN*SN==N){
			printf("No solution!\n");
			continue;
		}
		P = SN;
		Q = N - P*P;
		copy(x, SN);
		copy(y, 1);
		copy(x1, 1);
		copy(y1, 0);
		succ = 1;
		while(f*Q!=1) {
			if (x.top > 250 || y.top > 250){
				succ = 0;
				break;
			}
			a = (SN+P)/Q;
			P = a*Q-P;
			Q = (N-P*P)/Q;
			f *= -1;
			t = x;	
			mul(x, a);
			add(x, x1);
			x1 = t;
			t = y;
			mul(y, a);
			add(y, y1);
			y1 = t;
		}
		if (!succ){
			printf("No solution!\n");
			continue;
		}
		print(x);
		printf(" ");
		print(y);
		printf("\n");
	}
	return 0;
}
