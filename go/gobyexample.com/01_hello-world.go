package main

import "fmt"

func main() {
	fmt.Println("hello world")
}

/*
简单解读
像C和Java一样，一切从main函数开始，并且main函数所在的package（很多语言都有package概念，这是组织代码的一种典型方式，不赘述）也需要叫做main。import是导入其他的package，fmt是golang自带的一个可以用于输出的基础package，fmt.Println("hello world")是调用fmt包的Println方法，注意，此处方法的第一个字符是大写的，这是golang的规定，所有可以被外部包调用的方法都必须用大写字母开头，算是很多语言中public/protected/private的另一种实现方式：）
*/
