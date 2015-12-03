package cgo

import (
	"fmt"
	"testing"
)

func TestAll(t *testing.T) {
	Hw()
	fmt.Println("runtime_nano:", RuntimeNano())
}
