package main

import (
	"fmt"
	"time"
)

func cb(data interface{}) {
	fmt.Println(data)
}

func main() {
	t := time.NewTicker(time.Second).C

	for {
		<-t
		fmt.Println("hello")
	}
}
