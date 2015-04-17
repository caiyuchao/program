package main

import (
	"crypto/md5"
	"fmt"
	"github.com/stathat/consistent"
	"io"
	"strconv"
)

func Md5(raw string) string {
	h := md5.New()
	io.WriteString(h, raw)

	return fmt.Sprintf("%x", h.Sum(nil))
}
func main() {
	var c1 *consistent.Consistent = consistent.New()
	var c2 *consistent.Consistent = consistent.New()
	c1.NumberOfReplicas = 500
	c2.NumberOfReplicas = 500
	c1.Add("01")
	c1.Add("02")
	c1.Add("03")
	c1.Add("04")
	c1.Add("05")
	c1.Add("06")
	c1.Add("07")
	c1.Add("08")
	c1.Add("09")
	c1.Add("10")

	c2.Add("01")
	c2.Add("02")
	c2.Add("03")
	c2.Add("04")
	c2.Add("05")
	c2.Add("06")
	c2.Add("07")
	c2.Add("08")
	c2.Add("09")
	c2.Add("10")
	c2.Add("11")
	c2.Add("12")
	c2.Add("13")
	c2.Add("14")
	c2.Add("15")
	c2.Add("16")
	c2.Add("17")
	c2.Add("18")
	c2.Add("19")
	c2.Add("20")

	m1 := make(map[string]int)
	m2 := make(map[string]int)
	d := 0
	total := 0

	for i := 0; i < 10000000; i++ {
		m := strconv.Itoa(i)

		a, _ := c1.Get(m)
		if _, ok := m1[a]; !ok {
			m1[a] = 0
		} else {
			m1[a] = m1[a] + 1
		}

		b, _ := c2.Get(m)
		if _, ok := m2[b]; !ok {
			m2[b] = 0
		} else {
			m2[b] = m2[b] + 1
		}

		if a != b {
			d++
		}
		total++
	}
	fmt.Println("diff", d, "total", total)
	fmt.Println("m1", m1)
	fmt.Println("m2", m2)
}
