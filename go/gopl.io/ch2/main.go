package main

import "fmt"

func f() {}
func g() {}

var g = "g"

func main() {
	{
		f := "f"
		fmt.Println(f)
		fmt.Println(g)
		//fmt.Println(h)
		//Output:f
		//Output:g
	}
	{
		x := "hello!"
		for i := 0; i < len(x); i++ {
			x := x[i]
			if x != '!' {
				x := x + 'A' - 'a'
				fmt.Printf("%c", x) // "HELLO" (one letter periteration)
			}
		}
		fmt.Println()
		//Output:HELLO
	}
	{
		x := "hello"
		for _, x := range x {
			x := x + 'A' - 'a'
			fmt.Printf("%c", x) // "HELLO" (one letter per iteration)
		}
		fmt.Println()
		//Output:HELLO
	}
	{
		if x := f(); x == 0 {
			fmt.Println(x)
		} else if y := g(x); x == y {
			fmt.Println(x, y)
		} else {
			fmt.Println(x, y)
		}
		fmt.Println(x, y) // compile error: x and y are not visible here
	}
}
