package main

import (
	"bytes"
	"fmt"
)

func main() {
	b := make([]byte, 1024)
	m := make(map[string]interface{})

	copy(b[:], "www.sina.com.cn 1.1.1.1,1.1.1.2,1.1.1.3")
	i := bytes.IndexByte(b, ' ')
	a := bytes.Split(b[i+1:], []byte{','})
	host := make([]string, len(a))
	for k, v := range a {
		host[k] = string(v)
	}
	m[string(b[:i])] = host
	fmt.Printf("%v", m)

	//m[string(b[:i])] =
}
