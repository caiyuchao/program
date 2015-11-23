package main

/*
go run 34_log.go -log_dir="./34_log" -v=3
go run 34_log.go -log_dir="./34_log" -v=3 -vmodule=34_log=4
go run 34_log.go  -logtostderr=true -v=3
go run 34_log.go  -logtostderr=true -v=3 -vmodule=34_log=4
//	-logtostderr=false
//		Logs are written to standard error instead of to files.
//	-alsologtostderr=false
//		Logs are written to standard error as well as to files.
*/

import (
	"flag"
	"log"

	"github.com/golang/glog"
)

func main() {
	flag.Parse()
	glog.Info("hello")
	glog.V(2).Infof("info %d", 2)
	glog.V(3).Infof("info %d", 3)
	glog.V(4).Infof("info %d", 4)
	glog.Flush()

	log.SetFlags(log.Lshortfile)
	log.Print("hello world\n")
}
