package main

import (
	"fmt"
	"log"
	"reflect"
)

func main() {
	var x float64 = 1.0
	type asdf struct{}
	a := &asdf{}
	b := asdf{}
	var c interface{}
	c = 123
	d := uint32(c.(int))
	fmt.Println("type:", reflect.TypeOf(x))
	fmt.Println("type:", reflect.TypeOf(c))
	fmt.Println("type:", reflect.TypeOf(d))
	fmt.Println("type:", reflect.TypeOf(a))
	fmt.Println("type:", reflect.TypeOf(b))

	e := make([]int, 2)
	e[0] = 1
	e[1] = 2
	fmt.Println("type:", reflect.TypeOf(e))

	e1 := &e
	fmt.Println("e1", (*e1)[1])

	f := []int{1, 2}
	fmt.Println("type:", reflect.TypeOf(f))
	log.Println("type:", reflect.TypeOf(f))

}
