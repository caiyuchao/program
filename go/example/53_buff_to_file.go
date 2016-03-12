package main

import (
	"bytes"
	"fmt"
	"io"
)

func main() {
	// Initialize the encoder and decoder.  Normally enc and dec would be
	// bound to network connections and the encoder and decoder would
	// run in different processes.
	var i bytes.Buffer // Stand-in for a network connection
	var w io.Reader
	//var fd *os.File
	var fd *bytes.Buffer
	var buf []byte
	//fd = *os.File(&io)
	fd = &io
	fd.Write([]byte("hello,world"))
	fd.Read(buf)
	fmt.Printf("%s\n", string(buf))
}
