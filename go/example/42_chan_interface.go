package main

import (
	"fmt"
	"reflect"
	"time"
)

type foo_t struct {
	x int
	y int
}

var (
	int_c  chan int
	int_cc chan chan int
	i_c    chan interface{}
)

func main() {
	int_c = make(chan int, 1)
	int_cc = make(chan chan int, 1)
	i_c = make(chan interface{}, 1)

	go func() {
		for {
			select {
			case i := <-i_c:
				switch i.(type) {
				case int:
					fmt.Println("case int")
				case int32:
					fmt.Println("case int32")
				case foo_t:
					fmt.Println("case foo_t")
				default:
					fmt.Println("default type:", reflect.TypeOf(i).String()) // type: float64
				}
			}
		}
	}()

	i_c <- int(0)
	i_c <- int32(0)
	i_c <- uint32(0)
	i_c <- int_c
	i_c <- int_cc
	i_c <- foo_t{}
	time.Sleep(time.Second)
}
