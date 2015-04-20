package main

import (
	"flag"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"os/signal"
	"strconv"
	"syscall"
	"time"
)

var pid int
var CloseHttpChan, ClosedHttpChan chan int

type TcpKeepAliveListener struct {
	*net.TCPListener
}

func init() {
	CloseHttpChan = make(chan int, 1)
	ClosedHttpChan = make(chan int, 1)
}

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

func Fgetpid(file string) int {
	fd, err := os.Open(file)
	if err != nil {
		return 0
	}
	defer fd.Close()
	var pid int
	n, err := fmt.Fscanf(fd, "%d", &pid)
	if n != 1 || err != nil {
		return 0
	}
	return pid
}

func create_pidfile(pidfile string) error {
	os.Remove(pidfile)
	if f, e := os.OpenFile(pidfile, os.O_RDWR|os.O_CREATE,
		0644); e == nil {
		f.WriteString(strconv.Itoa(pid))
		f.Close()
		return nil
	} else {
		log.Println("openfile:", pidfile, e)
		return e
	}
}

func http_handle(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Hello ", pid)
}

func start_http() error {
	http.HandleFunc("/", http_handle)

	addr := "0.0.0.0:9090"
	s := &http.Server{
		Addr:           addr,
		MaxHeaderBytes: 1 << 30,
	}

	ln, err := net.Listen("tcp", addr)
	if err != nil {
		log.Println(pid, err)
		return err
	}
	l := ln.(*net.TCPListener)

	go s.Serve(TcpKeepAliveListener{l})
	log.Println(pid, "http listening port 9090")

	select {
	case <-CloseHttpChan:
		log.Println(pid, "http recv sigout and exit ...")
		l.Close()
		ClosedHttpChan <- 1
		return nil
	}
}

func start_signal() {

	args := os.Args
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM, syscall.SIGUSR2, syscall.SIGWINCH, syscall.SIGQUIT, syscall.SIGHUP)
	log.Println("register signal notify")

	for {
		s := <-sigs
		log.Println("recv", s)
		env := os.Environ()

		switch s {
		case syscall.SIGINT:
		case syscall.SIGTERM:
			os.Exit(1)
		case syscall.SIGUSR2:
			log.Println(pid, "rename .pid -> .oldbin and fork_execv new binary")
			os.Rename("15.pid", "15.oldbin")
			log.Println(pid, "close http Listener")
			CloseHttpChan <- 1
			<-ClosedHttpChan
			attr := &os.ProcAttr{
				Env:   env,
				Files: []*os.File{os.Stdin, os.Stdout, os.Stderr}}
			pid, err := os.StartProcess(args[0], args, attr)
			if err != nil {
				log.Println(err)
				os.Remove("15.pid")
			}
			log.Println("child pid", pid)
			os.Remove("15.oldbin")
			log.Println(pid, "exit")
			os.Exit(0)
		case syscall.SIGWINCH, syscall.SIGQUIT:
			log.Println("gracefull shut down")
			CloseHttpChan <- 1
			<-ClosedHttpChan
			time.Sleep(time.Second * 10) // do samethine
			os.Exit(0)
		case syscall.SIGHUP:
			log.Println("restart")
		}
	}
}

func main() {

	pwd, err := os.Getwd()
	if err != nil {
		log.Println(err)
		os.Exit(1)
	}
	f_chdir := flag.String("chdir", pwd, "a string")
	f_d := flag.Bool("d", true, "daemonize")
	f_i := flag.String("i", "/dev/null", "stdin")
	f_o := flag.String("o", "15.log", "stdout")
	f_e := flag.String("e", "15.log", "stderr")
	flag.Parse()

	log.SetFlags(log.LstdFlags | log.Lshortfile)

	if *f_d == true {
		daemon(*f_chdir, *f_i, *f_o, *f_e)
	}

	pid = Fgetpid("15.pid")
	if pid > 1 {
		if err := syscall.Kill(pid, 0); err == nil {
			log.Println(os.Args[0], pid, " alraedy running")
			os.Exit(1)
		}
	}

	pid = os.Getpid()
	create_pidfile("15.pid")

	go func() {
		if err := start_http(); err != nil {
			log.Println("exit:", pid, err)
			os.Exit(1)
		}
	}()

	start_signal()

}
