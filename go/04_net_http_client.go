package main

import (
	"fmt"
	"net/rpc"
)

const (
	URL = "127.0.0.1:12982"
)

type Args struct {
	A, B int
}

type Quotient struct {
	Quo, Rem int
}

func main() {

	client, err := rpc.DialHTTP("tcp", URL)
	if err != nil {
		fmt.Println(err.Error())
	}

	args := Args{2, 4}
	var reply int

	err = client.Call("Arith.Multiply", &args, &reply)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(reply)
	}

	err = client.Call("Arith.Multiply", &args, &reply)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(reply)
	}
}
