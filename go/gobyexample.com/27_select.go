// Go's _select_ lets you wait on multiple channel
// operations. Combining goroutines and channels with
// select is a powerful feature of Go.

package main

import (
	"fmt"
	"time"
)

func main() {

	// For our example we'll select across two channels.
	c1 := make(chan string)
	c2 := make(chan string)
	var cc1, cc2 int

	// Each channel will receive a value after some amount
	// of time, to simulate e.g. blocking RPC operations
	// executing in concurrent goroutines.
	go func() {
		//time.Sleep(time.Second * 1)
		for {
			c1 <- "one"
		}
	}()
	go func() {
		//time.Sleep(time.Second * 2)
		for {
			c2 <- "two"
		}
	}()

	// We'll use `select` to await both of these values
	// simultaneously, printing each one as it arrives.
	//for i := 0; i < 2; i++ {
	go func() {
		for {
			select {
			//case msg1 := <-c1:
			case <-c1:
				//fmt.Println("received", msg1)
				cc1 += 1
			//case msg2 := <-c2:
			case <-c2:
				//fmt.Println("received", msg2)
				cc2 += 1
			}
		}
	}()

	time.Sleep(time.Second)
	fmt.Println(cc1, cc2)
}
