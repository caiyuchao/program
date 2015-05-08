package cb

import (
	"fmt"
	"unsafe"
)

/*
#include "cb.h"
*/
import "C"

/*
C.char
C.schar (signed char)
C.uchar (unsigned char)
C.short
C.ushort (unsigned short)
C.int
C.uint (unsigned int)
C.long
C.ulong (unsigned long)
C.longlong (long long)
C.ulonglong (unsigned long long)
C.float
C.double
*/

//export go_callback_int
func go_callback_int(pfoo unsafe.Pointer, p1 C.ulong, p2 C.long) {
	foo := *(*func(C.ulong, C.long))(pfoo)
	foo(p1, p2)
}

func MyCallback(ts C.ulong, offset C.long) {
	fmt.Printf("callback with ts %08x offset %08x\n", ts, offset)
}

// we store it in a global variable so that the garbage collector
// doesn't clean up the memory for any temporary variables created.

func Example() {
	var MyCallbackFunc = MyCallback
	C.CallMyFunction(unsafe.Pointer(&MyCallbackFunc), C.ulonglong(0x0102030405060708))
}
