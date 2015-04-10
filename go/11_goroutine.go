package main

import (
	"fmt"
	"time"
)

func main() {
	go func() {
		fmt.Println("enter 1")
		defer fmt.Println("leave 1")
		go func() {
			fmt.Println("enter 2")
			defer fmt.Println("leave 2")
			time.Sleep(time.Second * 3)
		}()
	}()
	time.Sleep(time.Second * 2)
	fmt.Println("leave main")

}
