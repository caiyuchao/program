#!/bin/sh

go build 13_signal.go
./13_signal &
echo $! > run.pid
go run ./13_signal_kill.go

