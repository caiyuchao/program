// Copyright 2013 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"encoding/gob"
	"fmt"
	"io"
	"log"
	"os"
)

type Data struct {
	Time int64
	Data []byte
}

func (d Data) String() string {
	return fmt.Sprintf("time:%d data:%s", d.Time, string(d.Data))
}

func main() {
	var d Data
	log.SetFlags(log.Lshortfile)
	f, err := os.OpenFile("/tmp/52.dat", os.O_RDWR|os.O_CREATE, 0644)
	if err != nil {
		log.Fatal("openfile error:", err)
	}

	enc := gob.NewEncoder(f)
	dec := gob.NewDecoder(f) // Will read from network.

	d = Data{Time: 1, Data: []byte("hello")}
	err = enc.Encode(d)
	if err != nil {
		log.Fatal("encode error:", err)
	}
	d = Data{Time: 2, Data: []byte("world")}
	err = enc.Encode(d)
	if err != nil {
		log.Fatal("encode error:", err)
	}

	f.Seek(0, 0)

	de := func() {
		err := dec.Decode(&d)
		if err != nil {
			if err == io.EOF {
				fmt.Printf("get EOF, reset dec\n")
				f.Seek(0, 0)
				dec = gob.NewDecoder(f) // Will read from network.
			} else {
				fmt.Printf("fail %v\n", err)
			}
		} else {
			fmt.Printf("ok %s\n", d)
		}
	}

	de()
	de()
	de()
	de()
	de()
	de()

}
