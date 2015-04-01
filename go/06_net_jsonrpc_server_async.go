package main

import (
	"fmt"
	"net"
	"net/rpc"
	"net/rpc/jsonrpc"
	"sync"
	"time"
)

const (
	URL = ":12981"
)

type Args struct {
	A, B int
}

type Arith int

var seq int = 0
var mutex = &sync.Mutex{}

func (t *Arith) Multiply(args *Args, reply *int) error {
	mutex.Lock()
	seq = seq + 1
	s := seq
	mutex.Unlock()
	*reply = args.A*args.B + s
	time.Sleep(time.Millisecond * 500)
	return nil
}

func main() {

	arith := new(Arith)
	rpc.Register(arith)

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
