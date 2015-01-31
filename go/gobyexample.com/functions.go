package main

import "fmt"

// 下面是一个简单的例子，接收两个int参数，返回一个int结果
func plus(a int, b int) int {
	// Go语言需要显式写return语句，不会自动得返回函数中的
	// 最后一个表达式的值，请Ruby码农注意
	return a + b
}
func main() {
	// 如你所想，函数调用语法为：func_name(args)
	res := plus(1, 2)
	fmt.Println("1+2 =", res)
}
