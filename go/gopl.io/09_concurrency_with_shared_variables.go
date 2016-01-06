package main

import (
	"fmt"
	"log"
)

func init() {
	log.SetFlags(log.Lshortfile)
}

func section(s string) {
	fmt.Printf("## %s\n", s)
	log.SetPrefix(s + ": ")
}

func main() {
	fmt.Println("# 9. Concurrency with Shared Variables")
	section("9.1 Race Conditions")
	{
		// ch9/bank1 // use chan for goroutine Communication
	}
	section("9.2 Mutual Exclusion: sync.Mutex")
	{
		// ch9/bank2 // use chan as semaphore to lock
		// ch9/bank3 // use sync.Mutex to lock
	}
	section("9.3 Read/Write Mutexes: sync.RWMutex")
	{
		// sync.RWMutex
	}
	section("9.4 Memory Synchronization")
	{
		var x, y int
		go func() {
			x = 1                   // A1
			fmt.Print("y:", y, " ") // A2
		}()
		go func() {
			y = 1                   // B1
			fmt.Print("x:", x, " ") // B2
		}()
	}
	section("9.7 Example: Concurrent  Non-Blocking Cache")
	{
		// ch9/memo1  // lazyinit func() and cache
		// ch9/memo2  // use sync.Mutex
		// ch9/memo3
		// ch9/memo4  // use <-close(chan struct{}) for check ready

		// ch9/memo5 ****
		// Broadcast the ready condition.
		// close(e.ready)
		// Wait for the ready condition.
		// <-e.ready
	}
}
