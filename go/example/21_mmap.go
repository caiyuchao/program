package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"reflect"
	"syscall"
	"unsafe"
)

type mmap struct {
	buf      []byte
	buf_addr uintptr
	buf_len  int
	off      int // read at &buf[off], write at &buf[len(buf)]
}

func (m *mmap) Write(p []byte) (n int, err error) {
	if m.off >= len(m.buf) {
		// Buffer is empty, reset to recover space.
		m.off = 0
		if len(p) == 0 {
			return
		}
		return 0, io.EOF
	}
	n = copy(m.buf[m.off:], p)
	m.off += n
	return
}

func (m *mmap) WriteString(p string) (n int, err error) {
	if m.off >= len(m.buf) {
		// Buffer is empty, reset to recover space.
		m.off = 0
		if len(p) == 0 {
			return
		}
		return 0, io.EOF
	}
	n = copy(m.buf[m.off:], p)
	m.off += n
	return
}

func (m *mmap) Read(p []byte) (n int, err error) {
	if m.off >= len(m.buf) {
		// Buffer is empty, reset to recover space.
		m.off = 0
		if len(p) == 0 {
			return
		}
		return 0, io.EOF
	}
	n = copy(p, m.buf[m.off:])
	m.off += n
	return
}

func (m *mmap) String() string {
	return fmt.Sprintf("off:%d size:%d data:%x", m.off, m.buf_len, m.buf)
}

func (m *mmap) Reset() {
	m.off = 0
}

func (m *mmap) Seek(off int) bool {
	if off > 0 && off <= int(m.buf_len) {
		m.off = off
		return true
	} else {
		return false
	}
}

func (m *mmap) Dump() {
	log.Println(m)
}

func Newmmap(filename string) *mmap {
	var f *os.File
	var fi os.FileInfo
	var m mmap
	var err error

	if f, err = os.OpenFile(filename, os.O_RDWR|os.O_CREATE, 0644); err != nil {
		log.Fatalln(err)
	}
	//defer f.Close()
	if fi, err = f.Stat(); err != nil {
		log.Fatalln(err)
	}
	if m.buf, err = syscall.Mmap(int(f.Fd()), 0, int(fi.Size()), syscall.PROT_WRITE|syscall.PROT_READ, syscall.MAP_SHARED); err != nil {
		log.Fatalln(err)
	}
	fmt.Println(string(m.buf[:]))
	dh := (*reflect.SliceHeader)(unsafe.Pointer(&m.buf))
	m.buf_addr = dh.Data
	m.buf_len = dh.Len
	return &m
}

func main() {
	file := "21_mmap.go.txt"
	os.Remove(file)
	{
		f, _ := os.OpenFile(file, os.O_RDWR|os.O_CREATE, 0644)
		f.WriteString("hello world")
		f.Close()
	}
	{
		m := Newmmap(file)
		m.Dump()
		m.WriteString("1234")
		m.Dump()
		//flush
		//syscall.Syscall(syscall.SYS_MSYNC, dh.Data, uintptr(dh.Len), syscall.MS_SYNC)
		//unmap
		//syscall.Syscall(syscall.SYS_MUNMAP, dh.Data, uintptr(dh.Len), 0)
	}
	//mmap, err := Map(f, RDWR, 0)
}
