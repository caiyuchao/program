package main

import (
	"fmt"
)

func main() {
	// 字符串可以用 + 拼接
	fmt.Println("go" + "lang")

	// 整型、浮点型例子
	fmt.Println("1+1=", 1+1)
	fmt.Println("7.0/3.0=", 7.0/3.0)

	// bool型，支持bool运算
	fmt.Println(true && false)
	fmt.Println(true || false)
	fmt.Println(!true)
}
