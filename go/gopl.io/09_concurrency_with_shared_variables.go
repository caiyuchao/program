package main

import (
	"fmt"
	"log"
)

func init() {
	log.SetFlags(log.Lshortfile)
}

func section(s string) {
	fmt.Printf("## %s\n", s)
	log.SetPrefix(s + ": ")
}

func main() {
	fmt.Println("# 9. Concurrency	with	Shared	Variables")
	section("9.1 Race Conditions")
	{
		// ch9/bank1
	}
}
