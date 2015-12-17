package main

import "fmt"

func main() {
	var ar = [10]byte{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'}

	var a, b []byte

	a = ar[2:5]
	b = ar[:] // ar[0:len(ar)]
	ar[2] = 'a'

	fmt.Println(ar, a, b)
}
