//http://blog.golang.org/laws-of-reflection
//https://golang.org/pkg/reflect
package main

import (
	"fmt"
	"io"
	"os"
	"reflect"
)

func lab(name string) {
	fmt.Println("======" + name)
}
func main() {
	//tag
	{
		lab("1")
		type S struct {
			F string `species:"gopher" color:"blue"`
		}

		s := S{}
		st := reflect.TypeOf(s)
		field := st.Field(0)
		fmt.Println(field.Tag.Get("color"), field.Tag.Get("species"))
		fmt.Println(field.Tag)
	}
	{
		lab("2")
		var x float64 = 3.4
		fmt.Println("type:", reflect.TypeOf(x))  // type: float64
		fmt.Println("type:", reflect.ValueOf(x)) // value: <float64 Value>

		v := reflect.ValueOf(x)
		fmt.Println("type:", v.Type())                               // type: float64
		fmt.Println("kind is float64:", v.Kind() == reflect.Float64) // kind is float64: true
		fmt.Println("value:", v.Float())                             // value: 3.4
		fmt.Println("v.interface:", v.Interface(), "v:", v)          // v.interface: 3.4 v: <float64 Value>
	}
	{
		lab("3")
		var x uint8 = 'x'
		v := reflect.ValueOf(x)
		fmt.Println("type:", v.Type())                            // uint8.
		fmt.Println("kind is uint8: ", v.Kind() == reflect.Uint8) // true.
		x = uint8(v.Uint())                                       // v.Uint returns a uint64.
	}
	{
		lab("4")
		// As interface types are only used for static typing, a
		// common idiom to find the reflection Type for an interface
		// type Foo is to use a *Foo value.
		writerType := reflect.TypeOf((*io.Writer)(nil)).Elem()

		fileType := reflect.TypeOf((*os.File)(nil))
		fmt.Println(fileType.Implements(writerType))
	}
	{
		lab("5")
		type T struct {
			A int
			B string
		}
		t := T{23, "skidoo"}
		s := reflect.ValueOf(&t).Elem()
		typeOfT := s.Type()
		for i := 0; i < s.NumField(); i++ {
			f := s.Field(i)
			fmt.Printf("%d: %s %s = %v\n", i,
				typeOfT.Field(i).Name, f.Type(), f.Interface())
		}
	}
	{
		lab("6")
		// swap is the implementation passed to MakeFunc.
		// It must work in terms of reflect.Values so that it is possible
		// to write code without knowing beforehand what the types
		// will be.
		swap := func(in []reflect.Value) []reflect.Value {
			return []reflect.Value{in[1], in[0]}
		}

		// makeSwap expects fptr to be a pointer to a nil function.
		// It sets that pointer to a new function created with MakeFunc.
		// When the function is invoked, reflect turns the arguments
		// into Values, calls swap, and then turns swap's result slice
		// into the values returned by the new function.
		makeSwap := func(fptr interface{}) {
			// fptr is a pointer to a function.
			// Obtain the function value itself (likely nil) as a reflect.Value
			// so that we can query its type and then set the value.
			fn := reflect.ValueOf(fptr).Elem()

			// Make a function of the right type.
			v := reflect.MakeFunc(fn.Type(), swap)

			// Assign it to the value fn represents.
			fn.Set(v)
		}

		// Make and call a swap function for ints.
		var intSwap func(int, int) (int, int)
		makeSwap(&intSwap)
		fmt.Println(intSwap(0, 1))

		// Make and call a swap function for float64s.
		var floatSwap func(float64, float64) (float64, float64)
		makeSwap(&floatSwap)
		fmt.Println(floatSwap(2.72, 3.14))
	}

}
