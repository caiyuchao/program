// uint32_t round robin
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(){
	char s[] = "0123456789abcdefghijklmn";
	int j;
	uint32_t l, h, i = 0;
	i -= 10;
	l = i;

	for(j = 0; j < strlen(s); j++){
		i++;
		printf("j:%02d i:%08x offset:%02d c:%c\n", j, i, i-l, s[i-l]);
	}
	return 0;
}
