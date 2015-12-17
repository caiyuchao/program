package main

import (
	"fmt"
	"net"
	"net/rpc"
	"net/rpc/jsonrpc"
)

const (
	URL = ":12981"
)

type Se int

var seq int

func (t *Se) Pi(args *int, reply *int) error {
	*reply = seq
	seq++
	return nil
}

func main() {

	se := new(Se)
	rpc.Register(se)

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
