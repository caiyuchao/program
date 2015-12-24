package main

import (
	"crypto/sha256"
	"fmt"
	"time"
)

func main() {
	fmt.Println("# 4. Composite Types")

	fmt.Println("## 4.1 Arrays")
	{
		var a [3]int             // array of 3 integers
		fmt.Println(a[0])        // print the first element
		fmt.Println(a[len(a)-1]) // print the last element, a[2]
		// Print the indices and elements.
		for i, v := range a {
			fmt.Printf("%d %d\n", i, v)
		}
		// Print the elements only.
		for _, v := range a {
			fmt.Printf("%d\n", v)
		}
	}
	{
		//var q [3]int = [3]int{1, 2, 3}
		var r [3]int = [3]int{1, 2}
		fmt.Println(r[2]) // "0"
	}
	{
		q := [...]int{1, 2, 3}
		r := [...]int{99: -1}
		fmt.Printf("%T %T\n", q, r) // "[3]int [100]int"
	}
	{
		type Currency int
		const (
			USD Currency = iota
			EUR
			GBP
			RMB
		)
		symbol := [...]string{USD: "$", EUR: "€", GBP: "£", RMB: "¥"}
		fmt.Println(RMB, symbol[RMB]) // "3 ¥"
	}
	{
		a := [2]int{1, 2}
		b := [...]int{1, 2}
		c := [2]int{1, 3}
		fmt.Println(a == b, a == c, b == c) // "true false false"
		//d := [3]int{1, 2}
		//fmt.Println(a == d) // compile error: cannot compare [2]int == [3]int
	}
	{
		c1 := sha256.Sum256([]byte("x"))
		c2 := sha256.Sum256([]byte("X"))
		fmt.Printf("%x\n%x\n%t\n%T\n", c1, c2, c1 == c2, c1)
		// Output:
		// 2d711642b726b04401627ca9fbac32f5c8530fb1903cc4db02258717921a4881
		// 4b68ab3847feda7d6c62c1fbcbeebfa35eab7351ed5e78f4ddadea5df64b8015
		// false
		// [32]uint8
	}
	fmt.Println("## 4.2 Slices")
	{
		months := [...]string{
			1:  "January",
			2:  "February",
			3:  "march",
			4:  "April",
			5:  "May",
			6:  "June",
			7:  "July",
			8:  "August",
			9:  "September",
			10: "Octorber",
			11: "November",
			12: "December"}
		Q2 := months[4:7]
		summer := months[6:9]
		fmt.Println(Q2)     // ["April" "May" "June"]
		fmt.Println(summer) // ["June" "July" "August"]
		{
			for _, s := range summer {
				for _, q := range Q2 {
					if s == q {
						fmt.Printf("%s appears in both\n", s)
					}
				}
			}
		}
	}
	{
		var x []int
		x = append(x, 1)
		x = append(x, 2, 3)
		x = append(x, 4, 5, 6)
		x = append(x, x...) // append the slice x
		fmt.Println(x)      // "[1 2 3 4 5 6 1 2 3 4 5 6]"
	}
	fmt.Println("## 4.3 Maps")
	{
		/*
			ages := map[string]int{
				"alice":   31,
				"charlie": 34,
			}
		*/
		ages := make(map[string]int)
		ages["alice"] = 31
		ages["charlie"] = 34

		ages["alice"] = 32
		fmt.Println(ages["alice"]) // "32"

		delete(ages, "alice") // remove element ages["alice"]

		for name, age := range ages {
			fmt.Printf("%s\t%d\n", name, age)
		}
	}
	fmt.Println("## 4.4 Structs")
	{
		type Employee struct {
			ID        int
			Name      string
			Address   string
			DoB       time.Time
			Position  string
			Salary    int
			ManagerID int
		}
		var dilbert Employee
		dilbert.Salary -= 5000 // demoted, for writing too few lines of code
		position := &dilbert.Position
		*position = "Senior " + *position // promoted, for outsourcing to Elbonia
		fmt.Printf("%v\n", dilbert)

		var employeeOfTheMonth *Employee = &dilbert
		employeeOfTheMonth.Position += " (proactive team player)"
		(*employeeOfTheMonth).Position += " (proactive team player)"
		fmt.Printf("%v\n", dilbert)
	}
}
