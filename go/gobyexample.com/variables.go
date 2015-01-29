package main

import "fmt"

func main() {
	// var 声明一个或多个变量
	var a string = "initial"
	fmt.Println(a)
	// 可以一次性声明多个变量
	var b, c int = 1, 2
	fmt.Println(b, c)
	// 如果一个变量赋有初始值，go可以据此推断出变量类型
	var d = true
	fmt.Println(d)
	// 没有初始值的变量声明，go会自动为其设置零值
	// 比如此处，int的零值就是0（string的零值是空字符串""，bool的零值是false）
	var e int
	fmt.Println(e)
	// := 语法是声明并初始化一个变量的简写方式，
	// 此处实际是表示：var f string = "short"
	f := "short"
	fmt.Println(f)
}
