package main

import (
	"fmt"
	"time"
)

func main() {
	a := 1
	go func() {
		fmt.Println("go func a", a)
	}()
	{
		a := 2
		fmt.Println("sub a", a)
	}
	time.Sleep(time.Second)
	fmt.Println("main a", a)
}
