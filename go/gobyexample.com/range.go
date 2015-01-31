package main

import "fmt"

func main() {
	// 此处我们使用range来对slice中的数字做求和
	// 数组和slice是同样的工作方式
	nums := []int{2, 3, 4}
	sum := 0
	for _, num := range nums {
		sum += num
	}
	fmt.Println("sum:", sum)

	// range对array或slice做循环的时候，每次返回两个值：index，value
	// 上面的例子我们不需要index，所以使用下划线 _ 给忽略掉了
	// 不过有的时候我们确实需要index，比如下面的例子（牵强了，纯为举例）
	for i, num := range nums {
		if num == 3 {
			fmt.Println("index:", i)
		}
	}

	// 对一个map做range，会返回KV对
	kvs := map[string]string{"a": "apple", "b": "banana"}
	for k, v := range kvs {
		fmt.Printf("%s -> %s\n", k, v)
	}

	// 对字符串的range操作是基于Unicode码点。所以中文也不怕：）
	// 但是要注意index的值，此时的字符串可以看做是[]rune
	// index就是各个rune的第一个byte的索引，很绕是么，看两个例子
	fmt.Println("range string1>>>")
	for i, c := range "golang" {
		fmt.Println(i, c)
	}

	// “中”这个rune占了3个byte（第二、第三、第四个），所以索引是2
	// “国”这个rune占了3个byte（第五、第六、第七个），所以索引是5
	fmt.Println("range string2>>>")
	for i, c := range "go中国go" {
		fmt.Println(i, c)
	}
}
