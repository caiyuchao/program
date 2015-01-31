package main

import "fmt"

// 下面这个sum函数可以接收任意数目的数字作为参数
func sum(nums ...int) {
	fmt.Print(nums, " ")
	total := 0
	fmt.Printf("len(nums)=%d\n", len(nums))
	for _, num := range nums {
		total += num
	}
	fmt.Println(total)
}
func main() {
	// 可变参数函数可以使用通常的方法调用，传入多个参数值即可
	sum(1, 2)
	sum(1, 2, 3)

	// 如果你已经有一个包含有多个参数的slice，可以直接使用这个slice作为
	// 可变参数函数的参数，语法：func(slice...) 三个点...可以理解为把slice打散
	nums := []int{1, 2, 3, 4}
	sum(nums...)
}
