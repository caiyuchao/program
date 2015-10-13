package main

import (
	"fmt"
	"regexp"
)

func main() {
	matched, err := regexp.MatchString("foo.*", "seafood")
	fmt.Println(matched, err)
	matched, err = regexp.MatchString("bar.*", "seafood")
	fmt.Println(matched, err)
	matched, err = regexp.MatchString("a()b", "seafood")
	fmt.Println(matched, err)
	matched, err = regexp.MatchString("^(\\d{1,3}\\.){3}\\d{1,3}$", "a1.1.1.1")
	fmt.Println(matched, err)
}
