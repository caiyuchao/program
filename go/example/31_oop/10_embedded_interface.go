package main

import (
	/*
	 * 阅读下 heap.go
	 */
	"container/heap"
	"fmt"
)

type myHeap []int

func (h *myHeap) Less(i, j int) bool {
	return (*h)[i] < (*h)[j]
}

func (h *myHeap) Swap(i, j int) {
	(*h)[i], (*h)[j] = (*h)[j], (*h)[i]
}

func (h *myHeap) Len() int {
	return len(*h)
}

func (h *myHeap) Pop() (v interface{}) {
	*h, v = (*h)[:h.Len()-1], (*h)[h.Len()-1]
	return
}

func (h *myHeap) Push(v interface{}) {
	*h = append(*h, v.(int))
}

func (h myHeap) verify(i int) {
	n := h.Len()
	j1 := 2*i + 1
	j2 := 2*i + 2
	if j1 < n {
		if h.Less(j1, i) {
			fmt.Println("heap invariant invalidated [%d] = %d > [%d] = %d", i, h[i], j1, h[j1])
			return
		}
		h.verify(j1)
	}
	if j2 < n {
		if h.Less(j2, i) {
			fmt.Println("heap invariant invalidated [%d] = %d > [%d] = %d", i, h[i], j1, h[j2])
			return
		}
		h.verify(j2)
	}
}

func main() {
	h := new(myHeap)
	for i := 20; i > 0; i-- {
		h.Push(0) // all elements are the same
	}
	heap.Init(h)
	h.verify(0)

	for i := 1; h.Len() > 0; i++ {
		x := heap.Pop(h).(int)
		h.verify(0)
		if x != 0 {
			fmt.Println("%d.th pop got %d; want %d", i, x, 0)
		}
	}
}
