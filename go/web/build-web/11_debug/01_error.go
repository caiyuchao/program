package main

import (
	"errors"
	"fmt"
)

func foo(e string) error {
	return errors.New(e)
}
func main() {
	fmt.Println(foo("ooo"))
}
