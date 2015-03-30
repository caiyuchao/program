package main

import "fmt"

func main() {
	// 最基本的结构类型，只有一个条件，当while使
	i := 1
	for i <= 3 {
		fmt.Println(i)
		i = i + 1
	}
	// for经典结构 initial/condition/after
	for j := 7; j <= 9; j++ {
		fmt.Println(j)
	}
	// 如果for关键字后面没有条件语句，那就是死循环
	// 此时可以使用break跳出循环，或者使用return返回当前函数
	for {
		fmt.Println("loop")
		break
	}
}
