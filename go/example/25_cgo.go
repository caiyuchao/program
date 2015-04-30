package main

/*
#include "25_cgo_a.h"
#include "25_cgo_b.h"
*/

import "C"

func main() {
	C.foo()
}
