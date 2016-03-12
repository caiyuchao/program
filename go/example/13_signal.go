package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
)

func main() {
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM, syscall.SIGUSR1)
	//s := <-sigs
	//fmt.Println("got signal:", s)
	switch <-sigs {
	case syscall.SIGINT:
		fmt.Println("recv", syscall.SIGINT)
	case syscall.SIGTERM:
		fmt.Println("recv", syscall.SIGTERM)
	case syscall.SIGUSR1:
		fmt.Println("recv", syscall.SIGUSR1)
	}
}
