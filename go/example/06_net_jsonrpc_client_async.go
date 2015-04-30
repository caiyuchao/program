package main

import (
	"fmt"
	"net/rpc"
	"net/rpc/jsonrpc"
	"time"
)

const (
	URL = ":12981"
)

type Args struct {
	A, B int
}

type Quotient struct {
	Quo, Rem int
}

func main() {

	client, err := jsonrpc.Dial("tcp", URL)
	defer client.Close()

	if err != nil {
		fmt.Println(err)
	}

	args := Args{7, 2}

	var r [100]int

	for i := 0; i < 100; i++ {
		//go client.Call("Arith.Multiply", &args, &r[i])
		done := make(chan *rpc.Call, 1)
		call := client.Go("Arith.Multiply", &args, &r[i], done)
		go func(call *rpc.Call, done chan *rpc.Call, i int) {
			<-done
			if call.Error == nil {
				fmt.Println("No.", i, " return", r[i])
			} else {
				fmt.Println(call.Error)
			}
		}(call, done, i)
	}
	time.Sleep(time.Millisecond * 2000)
}
