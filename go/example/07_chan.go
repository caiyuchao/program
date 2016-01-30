package main

import (
	"fmt"
	"time"
)

func main() {
	c1 := make(chan int)
	cs := make(chan string)
	cb := make(chan bool)
	close(cs)
	close(cb)
	go func() {

		{
			// block
			i := <-c1
			fmt.Printf("%d\n", i)
		}

		{
			// block
			i, ok := <-c1
			fmt.Printf("%d %v\n", i, ok)
		}

		{
			i, ok := <-c1
			fmt.Printf("%d %v\n", i, ok)
		}

		{
			s, ok := <-cs
			fmt.Printf("\"%s\" %v\n", s, ok)
		}

		{
			b, ok := <-cb
			fmt.Printf("%v %v\n", b, ok)
		}

	}()
	// will deadlock
	c1 <- 1
	time.Sleep(time.Second)
	c1 <- 2
	close(c1)
	time.Sleep(time.Second)
}
