/* go run usock.go -s  // server mode
 * go run usock.go     // client mode
 * yubo@yubo.org
 */
package main

import (
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"net/rpc"
	"time"
)

const (
	URL = "./u.sock"
)

type Args struct {
	A, B int
}

type Quotient struct {
	Quo, Rem int
}

type FileByte struct {
	Filename string
	Body     []byte
}

type Arith int
type FileServer int

func (t *Arith) Multiply(args *Args, reply *int) error {
	*reply = args.A * args.B
	return nil
}

func (t *Arith) Divide(args *Args, quo *Quotient) error {
	if args.B == 0 {
		return errors.New("divide by zero")
	}
	quo.Quo = args.A / args.B
	quo.Rem = args.A % args.B
	return nil
}

func (t *FileServer) Get(filename string, file *FileByte) (err error) {
	if file.Body, err = ioutil.ReadFile(filename); err == nil {
		file.Filename = filename
	}
	return nil
}

func main() {
	server := flag.Bool("s", false, "server mode")
	flag.Parse()
	if *server {
		arith := new(Arith)
		rpc.Register(arith)

		fs := new(FileServer)
		rpc.Register(fs)

		//rpc.HandleHTTP()

		l, e := net.Listen("unix", URL)
		if e != nil {
			log.Fatal("listen error:", e)
		}

		//http.Serve(l, nil)
		{
			var tempDelay time.Duration
			for {
				conn, err := l.Accept()
				if err != nil {
					if tempDelay == 0 {
						tempDelay = 5 * time.Millisecond
					} else {
						tempDelay *= 2
					}
					if max := 1 * time.Second; tempDelay > max {
						tempDelay = max
					}
					time.Sleep(tempDelay)
					continue
				}
				tempDelay = 0
				go func() {
					rpc.ServeConn(conn)
				}()
			}
		}
	} else {
		client, err := rpc.Dial("unix", URL)
		if err != nil {
			log.Fatal("dialing:", err)
		}

		args := &Args{7, 8}
		//Then it can make a remote call:
		// Synchronous call
		{
			var reply int
			err = client.Call("Arith.Multiply", args, &reply)
			if err != nil {
				log.Fatal("arith error:", err)
			}
			fmt.Printf("Arith: %d*%d=%d", args.A, args.B, reply)
		}

		//or
		// Asynchronous call
		{
			quotient := new(Quotient)
			divCall := client.Go("Arith.Divide", args, quotient, nil)
			replyCall := <-divCall.Done // will be equal to divCall
			fmt.Println(replyCall)
			// check errors, print, etc.
		}

		// a.cap
		{
			var reply FileByte
			filename := "main.cap"
			err = client.Call("FileServer.Get", &filename, &reply)
			if err != nil {
				log.Fatalln(err)
			}
			if err = ioutil.WriteFile("get_"+filename, reply.Body, 0644); err != nil {
				log.Fatalln(err)
			}
			fmt.Println(reply.Filename)
		}
	}
}
