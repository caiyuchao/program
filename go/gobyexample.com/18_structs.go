package main

import "fmt"

// 我们定义一个结构体person，它有name和age两个字段
type person struct {
	name string
	age  int
}

func main() {
	// 这种语法可以创建一个person结构体
	fmt.Println(person{"Bob", 20})

	// 你可以采用命名字段的方式对结构体初始化
	fmt.Println(person{name: "Alice", age: 30})

	// 省略不赋值的字段将会用零值初始化，比如此处的age
	fmt.Println(person{name: "Fred"})

	// 使用 & 前缀可以获取结构体的指针
	fmt.Println(&person{name: "Ann", age: 40})

	// 通过点操作符访问结构体的字段
	s := person{name: "Sean", age: 50}
	fmt.Println(s.name)

	// 你也可以将点操作符应用于结构体指针来访问其字段，此时指针会自动被解引用
	// 此处不用如C语言那样采用->操作符
	sp := &s
	fmt.Println(sp.age)

	// 使用下面的方式对结构体字段赋值，说明结构体是可变的：）
	sp.age = 51
	fmt.Println(s.age)
}
