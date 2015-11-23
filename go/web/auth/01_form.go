package main

import (
	"fmt"
	"html/template"
	"log"
	"net/http"
)

const (
	tpl = `<html>
<head>
<title></title>
</head>
<body>
<form action="" method="post">
用户名:<input type="text" name="username" value="{{.Usr}}"><br />
密码:<input type="password" name="password" value="{{.Pwd}}"><br />
<input type="submit" value="登陆">
</form>
</body>
</html>`
)

type Login struct {
	Usr string
	Pwd string
}

func login(w http.ResponseWriter, r *http.Request) {
	fmt.Println("method:", r.Method) //获取请求的方法
	l := Login{}
	if r.Method == "GET" {
		t, _ := template.New("01_form").Parse(tpl)
		t.Execute(w, l)
	} else {
		//请求的是登陆数据，那么执行登陆的逻辑判断
		r.ParseForm()
		template.HTMLEscape(w, []byte(r.Form.Get("username"))) //输出到客户端
	}
}

func handle_null(w http.ResponseWriter, r *http.Request) {
}

func main() {
	http.HandleFunc("/", login)
	http.HandleFunc("/favicon.ico", handle_null)
	err := http.ListenAndServe(":9000", nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}
