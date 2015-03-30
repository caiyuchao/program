package main

import (
	"fmt"
	"strings"
	"time"
)

type req struct {
	cb   *chan string
	data string
}

var (
	request_chan chan *req
	update_chan  chan string
)

func foo(r *req) {
	*r.cb <- strings.Join([]string{"hello", r.data}, ":")
}

func worker() {
	for {
		select {
		case msg1 := <-request_chan:
			foo(msg1)
			fmt.Println("work requst_chan received", msg1.data)
		case msg2 := <-update_chan:
			fmt.Println("work update_chan received", msg2)
		}
	}
}

func main() {

	request_chan = make(chan *req)
	update_chan = make(chan string)

	go worker()

	t1 := time.NewTicker(time.Second).C
	go func() {
		for {
			select {
			case <-t1:
				fmt.Println("send update_chan")
				update_chan <- "update"
			}
		}
	}()

	t2 := time.NewTicker(time.Millisecond * 500)
	go func() {
		for range t2.C {
			resp := make(chan string)
			request := &req{
				cb:   &resp,
				data: "hello",
			}
			fmt.Println("send request_chan", request,
				"cb", request.cb,
				"data", request.data)
			request_chan <- request
			r := <-resp
			fmt.Println("received response", r)
		}
	}()
	time.Sleep(time.Millisecond * 5000)
}
