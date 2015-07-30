#include <stdio.h>

int main(int argc, char *argv[]){
	int i, v;
	if (argc < 2){
		fprintf(stderr, "Usage: %s number, ...\n", argv[0]);
	}
	printf("%-10s %-10s %-10s %-10s\n", "arg", "octal", "decimal", "hexadecimal");
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '0' && (argv[i][1] == 'x' || argv[i][1] == 'X')){
			sscanf(&argv[i][2], "%x", &v);
			printf("%-10s 0%-9o %-10u 0x%-8x\n", argv[i], v, v, v);
		}else if(argv[i][0] == '0'){
			sscanf(&argv[i][1], "%o", &v);
			printf("%-10s 0%-9o %-10u 0x%-8x\n", argv[i], v, v, v);
		}else{
			sscanf(argv[i], "%u", &v);
			printf("%-10s 0%-9o %-10u 0x%-8x\n", argv[i], v, v, v);
		}
	}
	return 0;
}
