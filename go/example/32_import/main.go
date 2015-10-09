package main

import (
	"./test01"
	/*
	* 引入该包，不能使用，just init()
	* 类似于 _, ok := a.b, 忽略a.b的值
	 */
	_ "./test02" //
)

func main() {
	test01.Echo()
	test02.Echo() //这一句会报错 undefined: test02
	return
}
