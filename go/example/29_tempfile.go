package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

func main() {
	if f, err := ioutil.TempFile("/tmp", "29_tempfile"); err == nil {
		f.WriteString("hello world")
		f.Close()
		fmt.Println(f.Name())
		os.Remove(f.Name())
		return
	}
}
