package main

import (
	"fmt"
	"os"
	"os/signal"
	"sync"
	"syscall"
	"time"
)

var (
	shutdown chan struct{}
	wg       sync.WaitGroup
)

func init() {
	shutdown = make(chan struct{}, 0)
}

func signal_register() {
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT)
	go func() {
		<-sigs
		fmt.Println("recv", syscall.SIGINT)
		close(shutdown)
	}()
}

func foo() {
	wg.Add(1)
	go func() {
		t := time.NewTicker(time.Second).C
		defer wg.Done()
		for {
			select {
			case <-t:
				fmt.Printf("hello from foo\n")
			case <-shutdown:
				time.Sleep(time.Second * 2)
				fmt.Printf("foo exit\n")
				return
			}
		}
	}()
}

func bar() {
	wg.Add(1)
	go func() {
		t := time.NewTicker(time.Second).C
		defer wg.Done()
		for {
			select {
			case <-t:
				fmt.Printf("hello from bar\n")
			case <-shutdown:
				time.Sleep(time.Second * 1)
				fmt.Printf("bar exit\n")
				return
			}
		}
	}()
}

func main() {
	signal_register()
	foo()
	bar()
	wg.Wait()
	fmt.Printf("main exit\n")
}
