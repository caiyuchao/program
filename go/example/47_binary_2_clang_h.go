package main

import (
	"fmt"
	"io/ioutil"
	"log"
)

const (
	filename = "/opt/public/demo.ts"
)

func main() {
	ctx, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Println(err)
	}
	l := len(ctx)
	fmt.Printf("const char ctx[] = {\n")
	for i := 0; i < l; i++ {
		fmt.Printf("0x%02x", ctx[i])
		if i+1 < l {
			fmt.Printf(",")
			if i%16 == 15 {
				fmt.Printf("\n")
			}
		}
	}
	fmt.Printf("};")
}
