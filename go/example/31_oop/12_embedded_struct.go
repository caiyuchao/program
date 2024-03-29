package main

import "fmt"

type Skills []string

type Human struct {
	name   string
	age    int
	weight int
}

type Student struct {
	Human     // struct as embedded field
	Skills    // string slice as embedded field
	int       // built-in type as embedded field
	specialty string
}

func main() {
	// initialize Student Jane
	jane := Student{Human: Human{"Jane", 35, 100}, specialty: "Biology"}
	// access fields
	fmt.Println("Her name is ", jane.Human.name)
	fmt.Println("Her name is ", jane.name)
	fmt.Println("Her age is ", jane.age)
	fmt.Println("Her weight is ", jane.weight)
	fmt.Println("Her specialty is ", jane.specialty)
	// modify value of skill field
	jane.Skills = []string{"anatomy"}
	fmt.Println("Her skills are ", jane.Skills)
	fmt.Println("She acquired two new ones ")
	jane.Skills = append(jane.Skills, "physics", "golang")
	fmt.Println("Her skills now are ", jane.Skills)
	// modify embedded field
	jane.int = 3
	fmt.Println("Her preferred number is ", jane.int)
}
