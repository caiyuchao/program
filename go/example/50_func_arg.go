package main

import "fmt"

type foo_t struct {
	s string
	i int
}

func foo() (f foo_t, err error) {
	f.s = "hello,world"
	f.i = 200
	return
}

func main() {
	fmt.Println(foo())
}
