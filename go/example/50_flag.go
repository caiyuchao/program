package main

import (
	"errors"
	"flag"
	"fmt"
	"strconv"
	"strings"

	"github.com/yubo/gotool/flags"
)

type foo_t struct {
	file string
	line int
}

var errTraceSyntax = errors.New("syntax error: expect file.go:234")

// String is part of the flag.Value interface.
func (f *foo_t) String() string {
	return fmt.Sprintf("%s:%d", f.file, f.line)
}

// Set is part of the flag.Value interface.
func (f *foo_t) Set(value string) error {
	if value == "" {
		// Unset.
		f.line = 0
		f.file = ""
	}
	fields := strings.Split(value, ":")
	if len(fields) != 2 {
		return errTraceSyntax
	}
	file, line := fields[0], fields[1]
	v, err := strconv.Atoi(line)
	if err != nil {
		return errTraceSyntax
	}
	f.line = v
	f.file = file
	return nil
}

var (
	b1       bool
	s1       string
	b2       *bool
	s2       *string
	f        foo_t
	sb1, sb2 bool
	ss1, ss2 string
)

func init() {
	{
		/* use var addr */
		flag.BoolVar(&b1, "b1", false, "a bool")
		flag.StringVar(&s1, "s1", "hello", "a string")
		flag.Var(&f, "foo", "a struct filename:line")
	}
	{
		/* get var addr */
		b2 = flag.Bool("b2", true, "a string")
		s2 = flag.String("s2", "world", "a string")
	}
	{
		/*
		 * sub command, like:
		 * git <command> [<args>]
		 */
		cmd1 := flags.NewCommand("cmd1", "cmd1,usage", flag.ExitOnError)
		cmd1.BoolVar(&sb1, "b", false, "a bool")
		cmd1.StringVar(&ss1, "s", "hello", "a string")

		cmd2 := flags.NewCommand("cmd2", "cmd2,usage", flag.ExitOnError)
		cmd2.BoolVar(&sb2, "b", false, "a bool")
		cmd2.StringVar(&ss2, "s", "hello", "a string")

	}
}

func main() {
	flags.Parse()

	fmt.Println("b1:", b1)
	fmt.Println("s1:", s1)
	fmt.Println("foo:", f)
	fmt.Println("b2:", *b2)
	fmt.Println("s2:", *s2)
	fmt.Println("cmd1.b:", sb1)
	fmt.Println("cmd1.s:", ss1)
	fmt.Println("cmd2.b:", sb2)
	fmt.Println("cmd2.s:", ss2)
}
