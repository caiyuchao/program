package main

import (
	"fmt"
	"log"
	"sync"
)

func init() {
	log.SetFlags(log.Lshortfile)
}

func section(s string) {
	fmt.Printf("## %s\n", s)
	log.SetPrefix(s + ": ")
}

func main() {
	fmt.Println("# 8. Goroutines and Channels")
	section("8.1 Goroutines")
	{
		// ch8/spinner
	}
	section("8.2 Example: Concurrent Clock Server")
	{
		// ch8/clock1
		// go run clock.go
		// nc localhost 8000

		// ch8/netcat1
		// ch8/clock2
	}
	section("8.3 Example: Concurrent Echo Server")
	{
		// ch8/reverb1
		// ch8/netcat2
		//
		// ch8/reverb2
	}
	section("8.4 Channels")
	{
		// ch8/netcat3
		// ch8/pipeline1
		// ch8/pipeline2
		// ch8/pipeline3
		/*
		   func mirroredQuery() string {
		    responses := make(chan string, 3)
		    go func() { responses <- request("asia.gopl.io") }()
		    go func() { responses <- request("europe.gopl.io") }()
		    go func() { responses <- request("americas.gopl.io") }()
		    return <-responses // return the quickest response
		   }
		*/
	}
	section("8.5 Looping in Parallel")
	{
		// ch8/thumbnail
		// closer
		var total int64
		var wg sync.WaitGroup
		sizes := make(chan int64)
		// ...

		go func() {
			wg.Wait()
			close(sizes)
		}()
		for size := range sizes {
			total += size
		}
	}
	section("8.6 Example: Concurrent Web Crawler")
	{
		// ch8/crawl1
		// ch8/crawl2
		// ch8/crawl3
	}
	section("8.7 Multiplexing with select")
	{
		// ch8/countdown1
		// ch8/countdown2
		// ch8/countdown3
	}
	section("8.8 Example: Concurrent Directory Traversal")
	{
		// ch8/du1  // chan close
		// ch8/du2  // braek loop label
		// ch8/du3  // WaitGroup
	}
	section("8.9 Cancellation")
	{
		// ch8/du4  // close chan by stdin; select define
	}
	section("8.10 Example: Chat Server")
	{
		// ch8/chat
	}

}
