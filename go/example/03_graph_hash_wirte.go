package main

import (
	"fmt"
	"hash/crc32"
	//	"strings"
	//"time"
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

func foo(now int64) {
	key := "hello"
	ts := now - ((now + int64(hashKey(key))) % cache_time) + cache_time

	/*
		write_offset := int64(hashKey(key))%cache_time - time%cache_time
		if write_offset < 0 {
			write_offset += cache_time
		}
		write_time := time + write_offset
	*/
	fmt.Println("now", now, "write_time", ts, "offset", ts-now)
}

func main() {
	//now := time.Now().Unix()
	var now int64 = 1428394120
	for i := 0; i < 180; i++ {
		foo(now + int64(i)*10)
	}

}
