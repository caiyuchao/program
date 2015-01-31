package main

import "fmt"
import "time"

func main() {
	// 这是一个基本的switch语句
	// 基本上支持switch语句的语言都支持这么写
	i := 2
	fmt.Print("write ", i, " as ")
	switch i {
	case 1:
		fmt.Println("one")
	case 2:
		fmt.Println("two")
	case 3:
		fmt.Println("three")
	}

	// 在同一个case语句中，我们可以用逗号分隔多个表达式
	// 这个例子中我们还使用了一个可选的default case语句
	// 显然，所有case都不满足的时候就走default，啰嗦啦：）
	switch time.Now().Weekday() {
	case time.Saturday, time.Sunday:
		fmt.Println("it's the weekend")
	default:
		fmt.Println("it's a weekday")
	}

	// switch后面无需跟表达式
	// 这种写法相当于if/else的另一种表达方式了
	// 这个例子也可以看出，case表达式也不一定非得是常量哦
	// 个人感觉这个特性还是很方便的，虽说习惯上还是用if/else来搞
	t := time.Now()
	switch {
	case t.Hour() < 12:
		fmt.Println("it's before noon")
	default:
		fmt.Println("it's after noon")
	}
}
