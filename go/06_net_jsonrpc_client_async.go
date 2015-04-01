package main

import (
	"fmt"
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
		go client.Call("Arith.Multiply", &args, &r[i])
	}
	time.Sleep(time.Millisecond * 2000)
	for i := 0; i < 100; i++ {
		fmt.Println(r[i])
	}
}
