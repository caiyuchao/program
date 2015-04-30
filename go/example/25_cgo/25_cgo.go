package cgo

/*
#include <stdio.h>
#include "25_cgo.h"
*/
import "C"

func Hw() {
	C.hw()
}
