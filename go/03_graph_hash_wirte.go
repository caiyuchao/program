package main

import (
	"fmt"
	"hash/crc32"
	//	"strings"
	"time"
)

const (
	cache_time int64 = 1800
)

func hashKey(key string) uint32 {
	if len(key) < 64 {
		var scratch [64]byte
		copy(scratch[:], key)
		return crc32.ChecksumIEEE(scratch[:len(key)])
	}
	return crc32.ChecksumIEEE([]byte(key))
}

func foo(time int64) {
	key := "hello"
	write_offset := int64(hashKey(key))%cache_time - time%cache_time
	if write_offset < 0 {
		write_offset += cache_time
	}
	write_time := time + write_offset
	fmt.Println("now", time, "write_time", write_time, "offset", write_offset)
}

func main() {
	time := time.Now().Unix()
	for i := 0; i < 180; i++ {
		foo(time + int64(i)*10)
	}

}
