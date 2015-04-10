package main

import (
	"fmt"
	"runtime"
	"sync"
	"time"
)

func main() {
	var mutex = &sync.Mutex{}
	runtime.GOMAXPROCS(1)
	go func() {
		mutex.Lock()
		fmt.Println("1 lock done")
		time.Sleep(time.Second * 3)
		fmt.Println("3 sleep done")
		mutex.Unlock()
		fmt.Println("1 unlock done")
	}()

	go func() {
		fmt.Println("2 lock before")
		mutex.Lock()
		fmt.Println("2 lock done")
		time.Sleep(time.Second)
		fmt.Println("2 sleep done")
		mutex.Unlock()
		fmt.Println("2 unlock done")
	}()
	time.Sleep(time.Second * 5)
}
