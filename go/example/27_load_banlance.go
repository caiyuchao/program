package main

import (
	"fmt"
	"runtime"
	"time"
)

const (
	task_size = 100000
	work_size = 30
	chan_size = 6
)

func main() {
	c := make(chan int, chan_size)
	out := make(chan int, work_size)
	numcpu := runtime.NumCPU()
	runtime.GOMAXPROCS(numcpu)

	for i := 0; i < work_size; i++ {
		go func(idx int) {
			n := 0
			for {
				select {
				case <-c:
					n += 1
				//	runtime.Gosched()
				case <-out:
					fmt.Println(idx, n)
					return
				}
			}
		}(i)
	}

	for i := 0; i < task_size; i++ {
		c <- i
	}
	time.Sleep(time.Second * 2)
	for i := 0; i < work_size; i++ {
		out <- i
	}
	time.Sleep(time.Second * 2)
}
