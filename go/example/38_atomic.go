package main

import (
	"fmt"
	"sync/atomic"
	"unsafe"
)

type t struct {
	x int
	y int
}

var p unsafe.Pointer

func foo() *t {
	return (*t)(atomic.LoadPointer(&p))
}

func bar() {
	atomic.StorePointer(&p, unsafe.Pointer(&t{x: 2, y: 3}))
}

func main() {
	p = unsafe.Pointer(&t{x: 1, y: 2})
	fmt.Println(foo(), p)
	bar()
	fmt.Println(foo(), p)
}
