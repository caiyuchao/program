package main

import (
	"fmt"
	"image/color"
	"log"
	"math"
)

func init() {
	log.SetFlags(log.Lshortfile)
}

func section(s string) {
	fmt.Printf("## %s\n", s)
	log.SetPrefix(s + ": ")
}

type Point struct{ X, Y float64 }

// traditional function
func Distance(p, q Point) float64 {
	return math.Hypot(q.X-p.X, q.Y-p.Y)
}

// same thing, but as a method of the Point type
func (p Point) Distance(q Point) float64 {
	return math.Hypot(q.X-p.X, q.Y-p.Y)
}

func (p *Point) ScaleBy(factor float64) {
	p.X *= factor
	p.Y *= factor
}

func (p Point) Add(q Point) Point { return Point{p.X + q.X, p.Y + q.Y} }
func (p Point) Sub(q Point) Point { return Point{p.X - q.X, p.Y - q.Y} }

// A Path is a journey connecting the points with straight lines.
type Path []Point

// Distance returns the distance traveled along the path.
func (path Path) Distance() float64 {
	sum := 0.0
	for i := range path {
		if i > 0 {
			sum += path[i-1].Distance(path[i])
		}
	}
	return sum
}

func (path Path) TranslateBy(offset Point, add bool) {
	var op func(p, q Point) Point
	if add {
		op = Point.Add
	} else {
		op = Point.Sub
	}
	for i := range path {
		// Call either path[i].Add(offset) or path[i].Sub(offset).
		path[i] = op(path[i], offset)
	}
}

// An IntList is a linked list of integers.
// A nil *IntList represents the empty list.
type IntList struct {
	Value int
	Tail  *IntList
}

// Sum returns the sum of the list elements.
func (list *IntList) Sum() int {
	if list == nil {
		return 0
	}
	return list.Value + list.Tail.Sum()
}

type ColoredPoint struct {
	Point
	Color color.RGBA
}

type ColoredPoint1 struct {
	*Point
	Color color.RGBA
}

func main() {
	fmt.Println("# 6. Methods")
	section("6.1 Method Declarations")
	{
		p := Point{1, 2}
		q := Point{4, 6}
		fmt.Println(Distance(p, q)) // "5", function call
		fmt.Println(p.Distance(q))  // "5", method call
		perim := Path{
			{1, 1},
			{5, 1},
			{5, 4},
			{1, 1},
		}
		fmt.Println(perim.Distance()) // "12"
	}
	section("6.2 Methods with a Pointer Receiver")
	{
		r := &Point{1, 2}
		r.ScaleBy(2)
		fmt.Println(*r) // "{2, 4}"
		// or
		p := Point{1, 2}
		q := Point{4, 6}
		pptr := &p
		pptr.ScaleBy(2)
		fmt.Println(p) // "{2, 4}"
		// or
		p = Point{1, 2}
		(&p).ScaleBy(2)
		fmt.Println(p) // "{2, 4}"
		// or
		// If the receiver p is a variable of type Point but the
		// method requires a *Point receiver, we can use this shorthand:
		p = Point{1, 2}
		p.ScaleBy(2)
		fmt.Println(p) // "{2, 4}"
		// --
		Point{1, 2}.Distance(q) // Point
		pptr.ScaleBy(2)         // *Point
		p.ScaleBy(2)            // implicit (&p)
		pptr.ScaleBy(2)         // implicit (*pptr)
	}
	section("6.3 Composing Types by Struct Embedding")
	red := color.RGBA{255, 0, 0, 255}
	blue := color.RGBA{0, 0, 255, 255}
	{
		var cp ColoredPoint
		cp.X = 1
		fmt.Println(cp.Point.X) // "1"
		cp.Point.Y = 2
		fmt.Println(cp.Y) // "2"

		var p = ColoredPoint{Point{1, 1}, red}
		var q = ColoredPoint{Point{5, 4}, blue}
		fmt.Println(p.Distance(q.Point)) // "5"
		p.ScaleBy(2)
		q.ScaleBy(2)
		fmt.Println(p.Distance(q.Point)) // "10"

		// p.Distance(q) //compile error: cannot use q(ColoredPoint) as Point
	}
	{
		p := ColoredPoint1{&Point{1, 1}, red}
		q := ColoredPoint1{&Point{5, 4}, blue}
		fmt.Println(p.Distance(*q.Point)) // "5"
		q.Point = p.Point                 // p and q now share the same Point
		p.ScaleBy(2)
		fmt.Println(*p.Point, *q.Point) // "{2 2} {2 2}"
	}
	section("# 6.4 Method Values and Expressions")
	{
		p := Point{1, 2}
		q := Point{4, 6}
		distanceFromP := p.Distance        // method value
		fmt.Println(distanceFromP(q))      // "5"
		var origin Point                   // {0, 0}
		fmt.Println(distanceFromP(origin)) // "2.23606797749979", √5
		scaleP := p.ScaleBy                // method value
		scaleP(2)                          // p becomes (2, 4)
		scaleP(3)                          // then (6, 12)
		scaleP(10)                         // then (60, 120)
	}
	{
		p := Point{1, 2}
		q := Point{4, 6}
		distance := Point.Distance   // method expression
		fmt.Println(distance(p, q))  // "5"
		fmt.Printf("%T\n", distance) // "func(Point, Point) float64"
		scale := (*Point).ScaleBy
		scale(&p, 2)
		fmt.Println(p)            // "{2 4}"
		fmt.Printf("%T\n", scale) // "func(*Point, float64)"
	}
	section("6.5 Example: Bit Vector Type")
	{
		fmt.Println("see intset")
	}
}
