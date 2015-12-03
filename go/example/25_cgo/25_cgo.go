package cgo

/*
#include <stdio.h>
#include "25_cgo.h"
*/
import "C"

func Hw() {
	C.hw()
}

func RuntimeNano() int64 {
	return int64(C.runtimeNano())
}
