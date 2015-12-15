package main

import "fmt"

func main() {
	{
		const (
			a uint32 = 0x0f
			b uint32 = 0xf0
			c uint32 = 0xff
		)
		fmt.Printf("%08x | %08x = %08x\n", a, b, a|b)
		fmt.Printf("%08x & %08x = %08x\n", a, b, a&b)
		fmt.Printf("%08x ^ %08x = %08x\n", a, c, a^c)
		fmt.Printf("^%08x = %08x\n", c, ^c)
		fmt.Printf("^%08x & %08x = %08x\n", b, c, ^b&c)
	}

	{
		const (
			bit00 uint32 = 1 << iota
			bit01
			bit02
		)
		fmt.Printf("%08x, %08x, %08x\n", bit00, bit01, bit02)
	}
}
