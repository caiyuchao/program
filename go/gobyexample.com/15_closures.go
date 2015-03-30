package main

import "fmt"

// 函数intSeq返回另一个函数，它是在intSeq的函数体内定义的匿名函数
// 返回的匿名函数用到了变量i，这形成了闭包
func intSeq() func() int {
	i := 0
	return func() int {
		i += 1
		return i
	}
}

func main() {
	// 我们调用intSeq，把返回的结果（一个函数）赋值给nextInt
	// 这个函数保持自己的变量i，变量i在每次调用nextInt的时候都会被更新
	nextInt := intSeq()

	// 我们调用几次nextInt，看看闭包的效果
	fmt.Println(nextInt())
	fmt.Println(nextInt())
	fmt.Println(nextInt())

	// 为了确认不同的闭包有不同的状态，我们重新创建并且测试一个新的返回函数。
	newInts := intSeq()
	fmt.Println(newInts())
}
