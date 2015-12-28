/* go run main.go -s true  // server mode
 * go run main.go          // client mode
 * use http/rpc maybe better
 * yubo@yubo.org
 */
package main

import (
	"encoding/base64"
	"flag"
	"fmt"
	"io/ioutil"
	"net"
	"net/rpc"
	"net/rpc/jsonrpc"
)

const (
	URL = ":12339"
)

type File64 struct {
	Filename string
	Body64   string
}

type FileServer int

func (t *FileServer) Get(filename string, file *File64) error {
	if f, err := ioutil.ReadFile(filename); err == nil {
		file.Body64 = base64.StdEncoding.EncodeToString(f)
		file.Filename = filename
	}
	return nil
}

func main() {
	server := flag.Bool("s", false, "server mode")
	flag.Parse()
	if *server {

		fs := new(FileServer)
		rpc.Register(fs)

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
	} else {
		var ctx []byte

		client, err := jsonrpc.Dial("tcp", URL)
		defer client.Close()

		if err != nil {
			fmt.Println(err)
		}

		args := "main.cap"
		var reply File64
		err = client.Call("FileServer.Get", &args, &reply)
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
}
