package main

import (
	"fmt"
)

type Cat struct {
}

var s []int

func main() {
	s = append(s, 1)
	fmt.Println(s)
}
