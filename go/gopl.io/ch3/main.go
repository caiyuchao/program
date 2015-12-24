package main

import (
	"fmt"
	"math"
	"time"
)

func main() {
	fmt.Println("## 3.2 Floating-Point Numbers")
	{
		var f float32 = 16777216 // 1 << 24
		fmt.Println(f == f+1)    // "true"!
	}
	{
		var z float64
		fmt.Println(z, -z, 1/z, -1/z, z/z) // "0 -0 +Inf -Inf NaN"
	}
	{
		nan := math.NaN()
		fmt.Println(nan == nan, nan < nan, nan > nan) // "false false false"
	}

	fmt.Println("## 3.3 complex numbers")
	{
		var x complex128 = complex(1, 2) // 1+2i
		var y complex128 = complex(3, 4) // 3+4i
		fmt.Println(x * y)               // "(-5+10i)"
		fmt.Println(real(x * y))         // "-5"
		fmt.Println(imag(x * y))         // "10"
		fmt.Println(1i * 1i)             // "(-1+0i)", i² = -1
	}

	fmt.Println("## 3.6 constants")
	{
		const noDelay time.Duration = 0
		const timeout = 5 * time.Minute
		fmt.Printf("%T %[1]v\n", noDelay)     // "time.Duration 0"
		fmt.Printf("%T %[1]v\n", timeout)     // "time.Duration 5m0s"
		fmt.Printf("%T %[1]v\n", time.Minute) // "time.Duration 1m0s"
	}
	{
		type Flags uint
		const (
			a = 1
			b
			c = 2
			d
		)
		fmt.Println(a, b, c, d) // "1 1 2 2"

		const (
			FlagUp           Flags = 1 << iota // is up
			FlagBroadcast                      // supports broadcast access capability
			FlagLoopback                       // is a loopback interface
			FlagPointToPoint                   // belongs to a point-topoint link
			FlagMulticast                      // supports multicast access capability
		)
		const (
			_   = 1 << (10 * iota)
			KiB // 1024
			MiB // 1048576
			GiB // 1073741824
			TiB // 1099511627776 (exceeds 1 << 32)
			PiB // 1125899906842624
			EiB // 1152921504606846976
			ZiB // 1180591620717411303424 (exceeds 1 << 64)
			YiB // 1208925819614629174706176
		)
		/*
			const (
				deadbeef = 0xdeadbeef        // untyped int with value 3735928559
				a        = uint32(deadbeef)  // uint32 with value 3735928559
				b        = float32(deadbeef) // float32 with value 3735928576 (rounded up)
				c        = float64(deadbeef) // float64 with value 3735928559 (exact)
				d        = int32(deadbeef)   // compile error: constant overflows int32
				e        = float64(1e309)    // compile error: constant overflows float64
				f        = uint(-1)          // compile error: constant underflows uint
			)
		*/
		fmt.Printf("%T\n", 0)      // "int"
		fmt.Printf("%T\n", 0.0)    // "float64"
		fmt.Printf("%T\n", 0i)     // "complex128"
		fmt.Printf("%T\n", '\000') // "int32" (rune)
	}
}
