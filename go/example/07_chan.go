package main

import (
	"fmt"
)

func main() {
	c1 := make(chan string)
	// will deadlock
	c1 <- "001"
	fmt.Println(c1.dataqsiz)
	go func() {
		for {
			fmt.Println("hello ", <-c1)
		}
	}()
	c1 <- "003"
	c1 <- "004"
}
