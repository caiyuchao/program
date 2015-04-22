package main

import (
	"fmt"
	"math"
)

func main() {
	var a string = "01234567"

	fmt.Println(^(^uint64(1)))
	fmt.Println(a == "h")
	fmt.Println(a[:3] == "012")
	fmt.Println(a[:3])
	fmt.Println(a[3:])
	math.NaN()
	{
		var a, c string
		var b int
		n, err := fmt.Sscanf("as 123 c3", "%s %d %s", &a, &b, &c)
		if err != nil {
			fmt.Println(n, err)
		} else {
			fmt.Println(n, a, b, c)
		}
	}
	{
		var a, c [10]byte
		var b int
		n, err := fmt.Sscanf("123\n", "%d\n", &b)
		if err != nil {
			fmt.Println(n, err)
		} else {
			fmt.Println(n, a, b, c)
		}
	}
}
