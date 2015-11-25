package main

import (
	"bytes"
	"html/template"
	"log"
	"net/http"
	"net/http/httptest"
	"net/url"
	"os"
	"time"
)

const (
	tpl = `cookie username: {{.Cname}}
method: {{.Method}}
username: {{.Usr}}
password: {{.Pwd}}
`
)

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

	t, _ := template.New("04_cookie").Parse(tpl)
	t.Execute(os.Stdout, l)
}

func main() {
	log.SetFlags(log.Llongfile)

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
