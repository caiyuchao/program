package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
)

type test struct {
	a int32
	b float32
	c []byte
	d uint8
	e string
}

func main() {
	buf := new(bytes.Buffer)
	var data = []interface{}{
		uint16(61374),
		int8(-54),
		uint8(254),
		uint8(0),
		uint8(1),
		uint8(0),
		[]byte("123412341234"[:5]),
	}
	for _, v := range data {
		err := binary.Write(buf, binary.LittleEndian, v)
		if err != nil {
			fmt.Println("binary.Write failed:", err)
		}
	}
	fmt.Printf("%x", buf.Bytes())
}
