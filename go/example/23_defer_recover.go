package main

import (
	"fmt"
	"runtime"
)

type foo_t struct {
	foo_int    int    `foo_int_d`
	foo_str    string `foo_str_d`
	foo_uint64 uint64 `foo_uint64_d`
}

func foo() (err error) {
	defer func() {
		err = fmt.Errorf("defer error")
	}()
	return fmt.Errorf("return error")
}

func bar() (err error) {
	defer func() {
		if r := recover(); r != nil {
			if _, ok := r.(runtime.Error); ok {
				panic(r)
			}
			err = r.(error)
		}
	}()
	return nil
}

func main() {
	fmt.Println(foo(), bar())
}
