package main

import (
	"encoding/base64"
	"fmt"
	"io/ioutil"
	"net"
	"net/rpc"
	"net/rpc/jsonrpc"
)

const (
	URL = ":12981"
)

type File64 struct {
	Filename string
	Body64   string
}

type ServerFile int

func (t *ServerFile) Get(filename string, file *File64) error {
	if f, err := ioutil.ReadFile(filename); err == nil {
		file.Body64 = base64.StdEncoding.EncodeToString(f)
		file.Filename = filename
	}
	return nil
}

func main() {

	server_file := new(ServerFile)
	rpc.Register(server_file)

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
