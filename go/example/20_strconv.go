package main

import (
	"fmt"
	"reflect"
	"strconv"
)

func d(v interface{}, err error) {
	if err != nil {
		fmt.Println(err)
	} else {
		fmt.Println(reflect.TypeOf(v), v)
	}
}

func main() {
	a := 1.1
	b := "1.2"
	d(a, nil)
	d(strconv.ParseFloat(b, 64))
}
