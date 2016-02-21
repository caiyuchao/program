package main

import (
	"fmt"
	"net"
)

func main() {
	ip := net.ParseIP("1.1.1.23")
	_, net, _ := net.ParseCIDR("0.0.0.0/0")
	fmt.Println(ip, net, net.Contains(ip))
}
