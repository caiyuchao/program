package main

import "fmt"

// 函数签名中的 （int，int） 表示这个函数返回两个int值
func vals() (int, int) {
	return 3, 7
}
func main() {
	// 此处我们调用这个多返回值的函数，使用两个变量来接收返回值
	a, b := vals()
	fmt.Println(a)
	fmt.Println(b)

	// 如果你只是需要多返回值的一个子集，使用下划线 _ 忽略不需要的值即可
	_, c := vals()
	fmt.Println(c)
}
