package main

import (
	"fmt"
	"time"
)

func cb(data interface{}) {
	fmt.Println(data)
}

func main() {
	time.Tick
	time.AfterFunc(2*time.Second, cb)
	time.Sleep(3 * time.Second)
}
