//��չŷ������㷨
//
/*
���gcd(a,b)=d�������m,n��ʹ��d = ma + nb��
�ƺ����ֹ�ϵΪa��b�������d��m��n��Ϊ���ϵ����
��d=1ʱ���� ma + nb = 1 ��
��ʱ���Կ���m��aģb�ĳ˷���Ԫ��n��bģa�ĳ˷���Ԫ��
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
