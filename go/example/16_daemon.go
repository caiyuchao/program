package main

import (
	"flag"
	"log"
	"os"
	"syscall"
)

func daemon(f_chdir string, f_i, f_o, f_e string) int {

	for i := 0; i < 2; i++ {
		ret, _, err := syscall.Syscall(syscall.SYS_FORK, 0, 0, 0)
		if err != 0 {
			return -1
		}
		switch ret {
		case 0:
			break
		default:
			os.Exit(0)
		}
	}

	if _, err := syscall.Setsid(); err != nil {
		return -1
	}

	os.Chdir(f_chdir)

	if f, e := os.OpenFile(f_i, os.O_RDWR, 0); e == nil {
		syscall.Dup2(int(f.Fd()), int(os.Stdin.Fd()))
	} else {
		log.Println("openfile:", f_i, e)
	}

	if f, e := os.OpenFile(f_o, os.O_RDWR|os.O_CREATE|os.O_APPEND,
		0644); e == nil {
		syscall.Dup2(int(f.Fd()), int(os.Stdout.Fd()))
	} else {
		log.Println("openfile:", f_o, e)
	}

	if f, e := os.OpenFile(f_e, os.O_RDWR|os.O_CREATE|os.O_APPEND,
		0644); e == nil {
		syscall.Dup2(int(f.Fd()), int(os.Stderr.Fd()))
	} else {
		log.Println("openfile:", f_e, e)
	}

	return 0
}

func main() {
	pwd, err := os.Getwd()
	if err != nil {
		log.Println(err)
		os.Exit(1)
	}
	f_chdir := flag.String("chdir", pwd, "a string")
	f_d := flag.Bool("d", false, "daemonize")
	f_i := flag.String("i", "/dev/null", "stdin")
	f_o := flag.String("o", "/dev/null", "stdout")
	f_e := flag.String("e", "/dev/null", "stderr")
	flag.Parse()

	log.Println(os.Getpid())
	if *f_d == true {
		daemon(*f_chdir, *f_i, *f_o, *f_e)
		log.Println(os.Getpid())
	}
	log.Println("exit")
}
