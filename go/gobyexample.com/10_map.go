package main

import "fmt"

func main() {
	// 使用内置make来创建一个空map: make(map[key-type]val-type).
	m := make(map[string]int)

	// 使用name[key] = val语法设置KV对
	m["k1"] = 7
	m["k2"] = 13

	// 可以使用Println对map做打印，它会打印出所有KV对，方便调试
	fmt.Println("map:", m)

	// 使用name[key]的方式获取某个key对应的value
	v1 := m["k1"]
	fmt.Println("v1: ", v1)

	// 内置的len可以返回map中KV对的数目
	fmt.Println("len:", len(m))

	// 内置的delete可以从map中删除某个KV对
	delete(m, "k2")
	fmt.Println("map:", m)

	// name[key]除了返回value之外，还有第二个返回值，这是可选的，看你是否接收
	// 第二个返回值是个bool类型，表明要获取的这个key是否存在
	// 这种方式可以消除“key缺失”和“key存在但value是零值”的歧义
	// This can be used to disambiguate between missing keys and keys with zero values like 0 or "".
	_, prs := m["k2"]
	fmt.Println("prs:", prs)

	// 同样的，我们可以在一行内声明并初始化一个新map
	n := map[string]int{"foo": 1, "bar": 2}
	fmt.Println("map:", n)
}
