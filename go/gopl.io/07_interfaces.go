package main

import (
	"bytes"
	"encoding/xml"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"syscall"
	"time"
)

func init() {
	log.SetFlags(log.Lshortfile)
}

func section(s string) {
	fmt.Printf("## %s\n", s)
	log.SetPrefix(s + ": ")
}

type ByteCounter int

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
		var err error = syscall.Errno(2)
		fmt.Println(err.Error()) // "no such file or directory"
		fmt.Println(err)         // "no such file or directory"
	}
	section("7.9 Example: Expression Evaluator")
	{
		//ch7/eval
		//ch7/surface
	}
	section("7.10 Type Assertions")
	{
		var w io.Writer
		w = os.Stdout
		rw := w.(io.ReadWriter) // success: *os.File has both Read and Write
		f := w.(*os.File)       // success: f == os.Stdout
		//c := w.(*bytes.Buffer)  // panic: interface holds *os.File, not * bytes.Buffer

		//w = new(ByteCounter)
		//rw = w.(io.ReadWriter) // panic: *ByteCounter has no Read method

		w = rw             // io.ReadWriter is assignable to io.Writer
		w = rw.(io.Writer) // fails only if rw == nil
		fmt.Printf("%#v, %#v\n", f, w)
	}
	{
		var w io.Writer = os.Stdout
		if f, ok := w.(*os.File); ok {
			fmt.Println(f, ok)
		} // success: ok, f == os.Stdout

		if b, ok := w.(*bytes.Buffer); ok {
			fmt.Println(b, ok)
		} // failure: !ok, b == nil
	}
	section("7.11 Discriminating Errors with Type Assertions")
	{
		//os.PathError.Error()
		_, err := os.Open("/no/such/file")
		fmt.Println(err) // "open /no/such/file: No such file or directory"
		fmt.Printf("%#v\n", err)
		//Output:
		//&os.PathError{Op:"open", Path:"/no/such/file", Err:0x2}
	}
	{
		// IsNotExist returns a boolean indicating whether the error is known to
		// report that a file or directory does not exist. It is satisfied by
		// ErrNotExist as well as some syscall errors.
		_, err := os.Open("/no/such/file")
		fmt.Println(os.IsNotExist(err)) // "true"
	}
	section("7.12 Querying Behaviors with Interface Type Assertions")
	{
		//see net/http
	}
	section("7.13 Type Switches")
	{
		var x int
		fmt.Println(sqlQuote(x))
	}
	section("7.14 token-based xml decoding")
	{
		// encoding/xml
		// ch7/xmlselect
		// ch1/fetch
		var _ xml.Attr
	}
}

func sqlQuote(x interface{}) string {
	switch x := x.(type) {
	case nil:
		return "NULL"
	case int, uint:
		return fmt.Sprintf("%d", x) // x has type interface{} here.
	case bool:
		if x {
			return "TRUE"
		}
		return "FALSE"
	case string:
		//return sqlQuoteString(x) // (not shown)
		return x // (not shown)
	default:
		panic(fmt.Sprintf("unexpected type %T: %v", x, x))
	}
}
