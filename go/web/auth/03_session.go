package main

import (
	"html/template"
	"log"
	"net/http"
	"sync"
	"time"
)

const (
	sid = "_03_session"
	tpl = `<html>
<head>
<title></title>
</head>
<body>
<form action="" method="post">
cookie username: {{.Cname}}<br />
method: {{.Method}} <br />
username: {{.Usr}} <br />
password: {{.Pwd}} <br />
用户名:<input type="text" name="username"><br />
密码:<input type="password" name="password"><br />
<input type="submit" value="登陆">
</form>
</body>
</html>`
)

type Session interface {
	sync.Mutex
	Set(key, value interface{}) error //set session value
	Get(key interface{}) interface{}  //get session value
	Start(w http.ResponseWriter, r *http.Request) error
	Delete(sid string) (Session, error)
}

func (s *Sessions) Set(key, value interface{}) error {
}

type Login struct {
	Token  string
	Usr    string
	Pwd    string
	Cname  string
	Method string
}

func login(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)
	r.ParseForm()
	l := Login{}
	if c, err := r.Cookie("cname"); err == nil {
		l.Cname = c.Value
	}
	l.Method = r.Method
	if r.Method == "POST" {
		//set cookie
		l.Usr = r.Form.Get("username")
		expiration := time.Now()
		expiration = expiration.AddDate(1, 0, 0) // 1 year
		/* get cookie
		 * GET / HTTP/1.1
		 * Host: 172.16.0.103:9000
		 * Connection: keep-alive
		 * User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.71 Safari/537.36
		 * Cookie: _gitlab_session=4831b77e63512a127cd213b7593c9621; cname=4444
		 */
		cookie := http.Cookie{Name: "cname", Value: l.Usr, Expires: expiration}
		l.Cname = l.Usr
		/* set cookie
		 * HTTP/1.1 200 OK
		 * Set-Cookie: cname=44444; Expires=Wed, 23 Nov 2016 08:16:29 UTC
		 * Date: Mon, 23 Nov 2015 08:16:29 GMT
		 * Content-Length: 376
		 * Content-Type: text/html; charset=utf-8
		 */
		http.SetCookie(w, &cookie)
	}

	t, _ := template.New("02_cookie").Parse(tpl)
	t.Execute(w, l)
}

func handle_null(w http.ResponseWriter, r *http.Request) {
}

func main() {
	log.SetFlags(log.Llongfile)
	http.HandleFunc("/", login)
	http.HandleFunc("/favicon.ico", handle_null)
	err := http.ListenAndServe(":9000", nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}
