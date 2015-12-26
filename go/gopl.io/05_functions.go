package main

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"math"
	"net/http"
	"os"
	"runtime"
	"time"
)

func hypot(x, y float64) float64 {
	return math.Sqrt(x*x + y*y)
}
func add(x int, y int) int   { return x + y }
func sub(x, y int) (z int)   { z = x - y; return }
func first(x int, _ int) int { return x }
func zero(int, int) int      { return 0 }

func f(x int) {
	log.Printf("f(%d)\n", x+0/x) //	panics	if	x	==	0
	defer log.Printf("defer	%d\n", x)
	f(x - 1)
}
func printStack() {
	var buf [4096]byte
	n := runtime.Stack(buf[:], false)
	os.Stdout.Write([]byte("\n----\n"))
	os.Stdout.Write(buf[:n])
	os.Stdout.Write([]byte("\n====\n"))
}

//	WaitForServer	attempts	to	contact	the	server	of	a	URL.
//	It	tries	for	one	minute	using	exponential	back-off.
//	It	reports	an	error	if	all	attempts	fail.
func WaitForServer(url string) error {
	const timeout = 2 * time.Second
	deadline := time.Now().Add(timeout)
	for tries := 0; time.Now().Before(deadline); tries++ {
		_, err := http.Head(url)
		if err == nil {
			return nil //	success
		}
		log.Printf("server not responding (%s);retrying...", err)
		time.Sleep(time.Second << uint(tries)) //	exponential	back-off
	}
	return fmt.Errorf("server %s failed to respond after %s", url, timeout)
}

func init() {
	log.SetFlags(log.Lshortfile)
}

func section(s string) {
	fmt.Printf("## %s\n", s)
	log.SetPrefix(s + ": ")
}

func main() {
	fmt.Println("# 5. Functions")
	section("5.1 Function Declarations")
	{
		fmt.Println(hypot(3, 4))  // "5"
		fmt.Printf("%T\n", add)   //	"func(int,	int)	int"
		fmt.Printf("%T\n", sub)   //	"func(int,	int)	int"
		fmt.Printf("%T\n", first) //	"func(int,	int)	int"
		fmt.Printf("%T\n", zero)  //	"func(int,	int)	int"
	}
	section("5.4 Errors")
	if false {
		url := "localhost"
		_, err := http.Get(url)
		if err != nil {
			log.Println(err)
			//return nil, err
		}
	}
	if false {
		url := "localhost"
		if err := WaitForServer(url); err != nil {
			//log.Fatalf("Site	is	down:	%v\n", err)
			log.Printf("Site is down: %v\n", err)
		}
	}
	if false {
		in := bufio.NewReader(os.Stdin)
		runes := make([]rune, 0)
		for {
			r, _, err := in.ReadRune()
			if err == io.EOF {
				break //	finished	reading
			}
			if err != nil {
				//return fmt.Errorf("read	failed:	%v", err)
				fmt.Printf("read	failed:	%v", err)
			}
			runes = append(runes, r)
			//	...use	r...
			log.Println(string(r))
		}
		log.Println(string(runes))
	}
	section("5.8 Deferred Function Call")
	{
		double := func(x int) (result int) {
			defer func() { fmt.Printf("double(%d) = %d\n", x, result) }()
			return x + x
		}
		_ = double(4)
		// Output:
		// "double(4) = 8"
		triple := func(x int) (result int) {
			defer func() { result += x }()
			return double(x)
		}
		fmt.Println(triple(4)) // "12"
	}
	section("5.9 Panic")
	if false {
		/*
			printStack := func() {
				var buf [4096]byte
				n := runtime.Stack(buf[:], false)
				os.Stdout.Write(buf[:n])
			}
			foo := func() { printStack() }
		*/
		func() {
			// panic之后，func stack的defer依次被调用，但没有将func从stack中弹出，
			// printStack依然可以访问到f(0), f(1), f(2), f(3)
			// *golang的实现就是golang标准
			defer printStack()
			f(3)
		}()
	}
	section("5.10 Recover")
	{
		// gopl.io/ch5/title3
	}
}
