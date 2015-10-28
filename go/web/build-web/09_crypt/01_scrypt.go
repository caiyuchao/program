package main

import "fmt"
import "scrypt"

func main() {
	salt := "hello"
	dk := scrypt.Key([]byte("some password"), []byte(salt), 16384, 8, 1, 32)
	fmt.Println(db)
}
