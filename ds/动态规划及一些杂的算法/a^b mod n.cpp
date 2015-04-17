//a^b mod n
//
#include<stdio.h>
int modExp(int a, int b, int n)
{
	int t = 1, y = a;
	while(b)
	{
		if(b&1)
			t = (t*y)%n;
		y = (y*y)%n;
		b >>= 1;
	}
	return t;
}

int main()
{
	int a, b, n;
	while(scanf("%d%d%d", &a, &b, &n)!=EOF)
		printf("%d\n", modExp(a, b, n));
	return 0;
}
