#include <poll.h>
#include <stdio.h>

int main(){
	poll(NULL, 0, 1100); // 1.100s
	return 0;
}
