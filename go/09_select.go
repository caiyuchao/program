package main

import "fmt"
import "sync"
import "runtime"

func TestPseudoRandomSend() {
	n := 100
	for _, chanCap := range []int{0, n} {
		c := make(chan int, chanCap)
		l := make([]int, n)
		var m sync.Mutex
		m.Lock()
		go func() {
			for i := 0; i < n; i++ {
				runtime.Gosched()
				l[i] = <-c
			}
			m.Unlock()
		}()
		for i := 0; i < n; i++ {
			select {
			case c <- 1:
			case c <- 0:
			}
		}
		m.Lock() // wait
		n0 := 0
		n1 := 0
		for _, i := range l {
			n0 += (i + 1) % 2
			n1 += i
		}
		if n0 <= n/10 || n1 <= n/10 {
			fmt.Println("Want pseudorandom, got %d zeros and %d ones (chan cap %d)", n0, n1, chanCap)
		}
	}
}

func main() {
	type mt struct{}
	m := &mt{}
	c := make(chan interface{}, 1)
	c <- m
	fmt.Println("1")
	select {
	case c <- m:
		fmt.Println("2")
	default:
		fmt.Println("3")
	}
	select {
	case c <- m:
		fmt.Println("4")
	case c <- &mt{}:
		fmt.Println("5")
	default:
		fmt.Println("6")
	}
	TestPseudoRandomSend()
}
