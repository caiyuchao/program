package main

import (
	"encoding/base64"
	"fmt"
	"io/ioutil"
	"net/rpc/jsonrpc"
)

const (
	URL = ":12981"
)

type File64 struct {
	Filename string
	Body64   string
}

func main() {
	var ctx []byte

	client, err := jsonrpc.Dial("tcp", URL)
	defer client.Close()

	if err != nil {
		fmt.Println(err)
	}

	args := "server.tar.gz"
	var reply File64
	err = client.Call("ServerFile.Get", &args, &reply)
	if err != nil {
		fmt.Println(err)
	}

	if ctx, err = base64.StdEncoding.DecodeString(reply.Body64); err != nil {
		fmt.Println(err)
	} else {
		if err = ioutil.WriteFile("get."+args,
			ctx, 0644); err != nil {
			fmt.Println(err)
		}
	}

	fmt.Println(reply.Filename)
}
