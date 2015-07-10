package main

import (
	"fmt"
	"net/http"

	"github.com/go-martini/martini"
)

func main() {
	m := martini.Classic()
	m.Use(Auth)

	m.Get("/", func() string {
		return "Hello world!"
	})

	m.Get("/hello/:name" /* Auth, */, func(params martini.Params) string {
		c := "hello "
		for k, v := range params {
			c += fmt.Sprintf(" %s=%s", k, v)
		}
		return c
	})
	m.Run()
}

func Auth(res http.ResponseWriter, req *http.Request) {
	if req.Header.Get("API-KEY") != "1234" {
		http.Error(res, "Nope", 401)
	}
}
