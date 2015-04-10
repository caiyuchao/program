package main

import (
	"fmt"
	"os"
	"syscall"
)

func getpid() int {
	fd, err := os.Open("run.pid")
	if err != nil {
		return 0
	}
	defer fd.Close()
	var pid int
	n, err := fmt.Fscanf(fd, "%d", &pid)
	if n != 1 || err != nil {
		return 0
	}
	return pid
}

func main() {
	pid := getpid()
	proc, _ := os.FindProcess(pid)
	proc.Signal(syscall.SIGUSR1)
	fmt.Println("send sigal", syscall.SIGUSR1, "pid", pid)
}
