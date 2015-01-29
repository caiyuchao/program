package main

import "fmt"

// 此处fact函数会一直调用自身，直到fact(0)
func fact(n int) int {
	if n == 0 {
		return 1
	}
	return n * fact(n-1)
}
func main() {
	fmt.Println(fact(7))
}
