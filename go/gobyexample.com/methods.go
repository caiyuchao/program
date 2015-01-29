package main

import "fmt"

type rect struct {
	width, height int
}

// func关键字和方法名之间的部分，我们称为方法的接收者（receiver），此处的接收者是*rect类型的
func (r *rect) area() int {
	return r.width * r.height
}

// 方法的receiver既可以是值类型，也可以是指针类型，此处是值类型的receiver
func (r rect) perim() int {
	return 2*r.width + 2*r.height
}

func main() {
	r := rect{width: 10, height: 5}

	// r是值类型，可以调用perim无可厚非，但是它也可以调用area
	fmt.Println("area: ", r.area())
	fmt.Println("perim:", r.perim())

	// rp是指针类型，可以调用area无可厚非，但是它也可以调用perim
	rp := &r
	fmt.Println("area: ", rp.area())
	fmt.Println("perim:", rp.perim())

	// Go在你执行方法调用的时候，自动对receiver做了转换：）
}
