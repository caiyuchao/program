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

func reconnection(client **rpc.Client) {
	var err error

	if *client != nil {
		(*client).Close()
	}

	*client, err = jsonrpc.Dial("tcp", URL)

	for err != nil {
		//danger!! block routine
		time.Sleep(time.Millisecond * 500)
		*client, err = jsonrpc.Dial("tcp", URL)
	}
}

func main() {
	var r int
	var client *rpc.Client = nil

	reconnection(&client)
	defer client.Close()

	for {
		if err := client.Call("Se.Pi", nil, &r); err != nil {
			fmt.Println(err)
			if err == rpc.ErrShutdown {
				reconnection(&client)
			}
		} else {
			fmt.Println(r)
		}

		time.Sleep(time.Second)
	}
}
