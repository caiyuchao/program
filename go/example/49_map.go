package main

import "fmt"

type foo_t struct {
	x int
	y int
}

func main() {
	{
		f := make(map[foo_t]int)
		a := foo_t{1, 2}
		f[a] = 3
		fmt.Println(f[foo_t{1, 2}])
		fmt.Println(f[foo_t{1, 1}])
		// output:
		// 3
		// 0
	}
	{
		f := make(map[*foo_t]int)
		a := &foo_t{1, 2}
		f[a] = 3
		fmt.Println(f[a])
		fmt.Println(f[&foo_t{1, 2}])
		// output:
		// 3
		// 0
	}
	{
		f := make(map[interface{}]int)
		a := foo_t{1, 2}
		f[a] = 3
		f[&a] = 4
		fmt.Println(f[a])
		fmt.Println(f[&a])
		// output:
		// 3
		// 4
	}
}
