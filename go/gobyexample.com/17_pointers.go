package main

import "fmt"

// zeroval的参数是int类型的，参数通过值传递的方式传入函数
// zeroval将会得到ival的一个拷贝，所以函数内部对ival再怎么改变赋值，都不会影响外层的值
func zeroval(ival int) {
	ival = 0
}

// 相比而言,zeroptr有一个*int类型的参数，这是int类型的指针
// 与C语言一样*iptr可以存取该指针对应的值
func zeroptr(iptr *int) {
	*iptr = 0
}

func main() {
	i := 1
	fmt.Println("initial:", i)
	zeroval(i)
	fmt.Println("zeroval:", i)
	// 与C语言一样，&i可以获得i的内存地址，即i的指针
	zeroptr(&i)
	fmt.Println("zeroptr:", i)
	// 指针可以被直接打印输出
	fmt.Println("pointer:", &i)
}
