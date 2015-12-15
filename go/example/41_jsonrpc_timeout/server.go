package main

import (
	"fmt"
	"math/rand"
	"net"
	"net/rpc"
	"net/rpc/jsonrpc"
	"time"
)

const (
	URL = ":12981"
)

type Sleep int

var seq int

func (t *Sleep) S10(args *int, reply *int) error {
	time.Sleep(time.Second * time.Duration(rand.Intn(10)))
	*reply = seq
	seq++
	return nil
}

func main() {

	sleep := new(Sleep)
	rpc.Register(sleep)

	tcpAddr, err := net.ResolveTCPAddr("tcp", URL)
	if err != nil {
		fmt.Println(err)
	}
	listener, err := net.ListenTCP("tcp", tcpAddr)

	for {
		conn, err := listener.Accept()
		if err != nil {
			continue
		}
		go jsonrpc.ServeConn(conn)
	}
}
