//Source Code-27 RMQ*
			
			
#include<stdio.h>
			
struct TNode
{
	int l;
	int r;
	int v;
};
		
TNode T[1 << 20];

int TQuery(int n, int left, int right)
{
	int lc = (n << 1) + 1;
	if(left == T[n].l && right == T[n].r)
		return T[n].v;
	else if(T[lc + 1].l <= left)
		return TQuery(lc + 1, left, right);
	else if(right <= T[lc].r)
		return TQuery(lc, left, right);
	else
	{
		left = TQuery(lc, left, T[lc].r);
		right = TQuery(lc + 1, T[lc + 1].l, right);
		return left > right ? left : right;
	}
}
		
int TUpdate(int n, int left, int right, int value)
{
	int lc = (n << 1) + 1, t;
	if(T[n].l == left && T[n].r == right)
		T[n].v = value;
	else
	{
		if(T[lc + 1].l <= left)
		{
			t = TUpdate(lc + 1, left, right, value);
			T[n].v = t > T[lc].v ? t : T[lc].v;
		}
		else if(right <= T[lc].r)
		{
			t = TUpdate(lc, left, right, value);
			T[n].v = t > T[lc + 1].v ? t : T[lc + 1].v;
		}
		else
		{
			T[n].v = TUpdate(lc, left, T[lc].r, value);
			t = TUpdate(lc + 1, T[lc + 1].l, right, value);
			T[n].v = t > T[n].v ? t : T[n].v;
		}
	}
    return T[n].v;
}

int main()
{
	int n, m, i, j, k, v, l, r;
	while(scanf("%d%d", &n, &m) == 2)
	{
		for(j = 0, i = 1; i < n; i <<= 1)
			j += i;
				
		k = i;
		// 在此建立线段树，要求0为根节点
		// 具体代码略:-)
		for(i = 0; i < m; ++i)
		{
			getchar();
			v = getchar();
			scanf("%d%d", &l, &r);
			if(v == 'Q')
				printf("%d\n", TQuery(0, l, r));
			else
				TUpdate(0, l, l, r);
		}
    }
    return 0;
}

