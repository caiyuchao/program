package main

import (
	"errors"
	"fmt"
	"net/rpc"
	"net/rpc/jsonrpc"
	"time"
)

const (
	URL = ":12981"
)

func jsonrpc_call(client *rpc.Client, method string, args interface{}, reply interface{}, timeout time.Duration) error {
	done := make(chan *rpc.Call, 1)
	client.Go(method, args, reply, done)
	select {
	case <-time.After(timeout):
		return errors.New("timeout")
	case call := <-done:
		if call.Error == nil {
			return nil
		} else {
			return call.Error
		}
	}
}

func main() {

	client, err := jsonrpc.Dial("tcp", URL)
	defer client.Close()

	if err != nil {
		fmt.Println(err)
	}
	var r [100]int

	for i := 0; i < 100; i++ {
		go func(i int) {
			err := jsonrpc_call(client, "Sleep.S10", nil, &r[i], time.Second*3)
			fmt.Println(i, r[i], err)
		}(i)
	}
	time.Sleep(time.Second * 15)
}
