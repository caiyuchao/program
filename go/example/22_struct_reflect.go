package main

import (
	"log"
	"reflect"
)

type foo_union_t struct {
	foo_int     int
	foo_float64 float64
	_name       string
	_size       int
}

type foo_struct_t struct {
	foo_int     int
	foo_float64 float64
}

type foo_t struct {
	foo_int    int
	foo_str    string
	foo_uint64 uint64
	foo_union  foo_union_t  `type:"union"`
	foo_struct foo_struct_t `type:"struct"`
}

type foo_error struct {
	Type reflect.Type
}

func (e *foo_error) Error() string {
	if e.Type == nil {
		return "type is nil"
	}

	if e.Type.Kind() != reflect.Ptr {
		return "non pointer " + e.Type.String()
	}

	return "nil" + e.Type.String()
}

func main() {
	/*
		v := foo_t{
			foo_int:    1234,
			foo_str:    "hello world",
			foo_uint64: 12341234,
			foo_union: foo_union_t{foo_int: 1234,
				foo_float64: 1.234,
				_name:       "foo_int",
			},
			foo_struct: foo_struct_t{
				foo_int:     1234,
				foo_float64: 1.234,
			},
		}
	*/
	type T struct {
		A int
		b string
	}

	Abc := T{23, "hello"}

	log.SetFlags(log.LstdFlags | log.Lshortfile)
	rv := reflect.ValueOf(&Abc).Elem()
	rt := rv.Type()
	for i := 0; i < rv.NumField(); i++ {
		f := rv.Field(i)
		switch rv.Field(i).Kind() {
		case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
			fallthrough
		case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
		case reflect.Slice, reflect.Array:
		case reflect.Struct:
		default:
		}
		log.Println(i, "name", rt.Field(i).Name, f.Type(), f.Kind(), f.Interface())

	}

}
