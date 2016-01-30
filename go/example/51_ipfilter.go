package main

import (
	"fmt"
	"net"
)

func main() {
	ip := net.ParseIP("1.1.1.23")
	_, net, _ := net.ParseCIDR("1.1.1.1/24")
	fmt.Println(ip, net, net.Contains(ip))
}
