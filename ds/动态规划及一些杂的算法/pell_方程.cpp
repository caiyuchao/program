//pell·½³Ì
#include<math.h>
#include<stdio.h>
int main()
{
	int N;
    int a, P, Q, SN;
    long long x, y, x1, y1, t;
    while(scanf("%d", &N), N){
		int f = -1;
		SN = (int)sqrt(N);
		if(SN*SN==N){
			printf("No solution!\n");
			continue;
		}
		P = SN;
		Q = N - P*P;
		x = SN;
		y = 1;
		x1 = 1;
		y1 = 0;
		while(f*Q!=1) {
			a = (int)((SN+P)/Q);
			P = a*Q-P;
			Q = (N-P*P)/Q;
			f *= -1;
			t = x;
			x = a*x + x1;
			x1 = t;
			t = y;
			y = a*y + y1;
		 	y1 = t;
		}
		printf("x = %I64d", x);
		printf("y = %I64d\n", y);
	}
	return 0;
}


