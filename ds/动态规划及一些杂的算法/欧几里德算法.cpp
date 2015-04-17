//扩展欧几里德算法
//
/*
如果gcd(a,b)=d，则存在m,n，使得d = ma + nb，
称呼这种关系为a、b组合整数d，m，n称为组合系数。
当d=1时，有 ma + nb = 1 ，
此时可以看出m是a模b的乘法逆元，n是b模a的乘法逆元。
*/
#include<stdio.h>
int x, y;
int euclid(int a, int b){
	int res, t;
	if(b==0){
		res = a;
		x = 1;
		y = 0;
	}
	else{
		res = euclid(b, a%b);
		t = x;
		x = y;
		y = t-(a/b)*y;
	}
	return res;
}
int main(){
	int a, b, res;
	while(scanf("%d%d", &a, &b)!=EOF){
		res = euclid(a, b);
		printf("%d = %d*a + %d*b\n", res, x, y);
	}
    return 0;
}
