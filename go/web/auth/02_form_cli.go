package main

import (
	"bytes"
	"fmt"
	"net/http"
	"net/http/httptest"
	"net/url"
	"os"
	"text/template"
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
		t, _ := template.New("02_form").Parse(tpl)
		t.Execute(os.Stdout, l)
	} else {
		r.ParseForm()
		//fmt.Println(r)
		template.HTMLEscape(os.Stdout, []byte(r.Form.Get("username")+"\n")) //输出到客户端
	}
}

func main() {
	v := url.Values{}
	v.Set("username", "test")
	v.Add("password", "1234")
	body := v.Encode()

	r, _ := http.NewRequest("POST", "/", bytes.NewBufferString(body))
	r.Header.Add("Content-Type", "application/x-www-form-urlencoded")
	//r.Header.Add("Content-Length", strconv.Itoa(len(en)))
	w := httptest.NewRecorder()

	login(w, r)
}
