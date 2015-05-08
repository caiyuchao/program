#include "cb.h"
#include "_cgo_export.h"
#include <stdint.h>
#include <stdio.h>

void CallMyFunction(void* pfoo, unsigned long long recv) {
	printf("recv: 0x%016llx\n", recv);
	go_callback_int(pfoo, recv>>32, recv & 0xFFFFFFFF);
}
