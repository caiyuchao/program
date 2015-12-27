package main

import (
	"bytes"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"time"
)

func init() {
	log.SetFlags(log.Lshortfile)
}

func section(s string) {
	fmt.Printf("## %s\n", s)
	log.SetPrefix(s + ": ")
}

func main() {
	fmt.Println("# 7. Interfaces")
	section("7.1 Interfaces as Contracts")
	{
		//see
		var _ io.Writer
	}
	section("7.2 Interface Types")
	{
		//see
		var _ io.Reader
		var _ io.Closer
		var _ io.ReadWriter
		var _ io.ReadWriteCloser
	}
	section("7.3 Interface Satisfaction")
	{
		var w io.Writer
		w = os.Stdout         // OK: *os.File has Write method
		w = new(bytes.Buffer) // OK: *bytes.Buffer has Write method
		//w = time.Second       // compile error: time.Duration lacks Write method
		var rwc io.ReadWriteCloser
		rwc = os.Stdout // OK: *os.File has Read, Write, Close methods
		//rwc = new(bytes.Buffer) // compile error: *bytes.Buffer lacks Close method
		w = rwc // OK: io.ReadWriteCloser has Write method
		//rwc = w                 // compile error: io.Writer lacks Close method
		fmt.Printf("%T\n", w)

		var _ fmt.Stringer
	}
	section("7.4 Parsing Flags with flag.Value")
	{
		//see
		var _ flag.Value
		flag.Duration("period", 1*time.Second, "sleep period")
		flag.Parse()
	}
	section("7.5 Interface Values")
	{
		var w io.Writer
		fmt.Printf("%T\n", w) // "<nil>"
		w = os.Stdout
		fmt.Printf("%T\n", w) // "*os.File"
		w = new(bytes.Buffer)
		fmt.Printf("%T\n", w) // "*bytes.Buffer"
	}
	section("7.6 Sorting with sort.Interface")
	{
		//see ch7/sorting/main.go
	}
	section("7.7 The http.Handler Interface")
	{
		//see
		//ch7/http1
		//ch7/http2
		//ch7/http3
		//ch7/http3a
		//ch7/http4
	}
	section("7.8 The error Interface")
	{
	}
}
