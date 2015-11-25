package main

import (
	"fmt"
	"html/template"
	"log"
	"net/http"
	"time"
)

const (
	sess_key = "_03_session"
	usr      = "test"
	pwd      = "1234"
	tpl      = `<html>
<head>
<title></title>
</head>
<body>
method: {{.Method}} <br />
{{if .Err}}
	<pre>{{.Err}}</pre><br />
{{else}}
	{{if .Sess.Usr}}
		Sid: {{.Sess.Sid}} <br />
		username: {{.Sess.Usr}} <br />
		firstname: {{.Sess.Fname}} <br />
		lastname: {{.Sess.Lname}} <br />
		<a href="/logout">logout</a><br />
	{{else}}
		<form action="/" method="post">
	   		username:<input type="text" name="username"><br />
	   		password:<input type="password" name="password"><br />
	   		firstname:<input type="text" name="firstname"><br />
	   		lastname:<input type="text" name="lastname"><br />
	   		<input type="submit"><br />
		</form>
	{{end}}
{{end}}
<a href="/">index</a><br />
</body>
</html>`
)

// todo add sync.Mutex
var global_session map[string]*Session

type Session struct {
	Sid   string
	Usr   string
	Fname string
	Lname string
}

type Draw struct {
	Sess   Session
	Method string
	Err    string
}

func init() {
	global_session = make(map[string]*Session)
}

func sess_start(w http.ResponseWriter, r *http.Request) (sid string, err error) {
	if c, err := r.Cookie(sess_key); err == nil {
		//todo check Expires
		sid = c.Value
	} else {
		//todo key should complex
		expiration := time.Now()
		expiration = expiration.AddDate(1, 0, 0) // 1 year
		sid = fmt.Sprintf("%d-%d", time.Now().Unix(), len(global_session))
		cookie := http.Cookie{Name: sess_key, Value: sid, Expires: expiration}
		http.SetCookie(w, &cookie)
	}
	return
}

func login(w http.ResponseWriter, r *http.Request) {
	var (
		sess *Session
		draw Draw
	)

	log.Println("login", r.Method)
	draw.Method = r.Method

	r.ParseForm()
	sid, _ := sess_start(w, r)

	if s, ok := global_session[sid]; ok {
		sess = s
	} else {
		sess = &Session{Sid: sid}
		global_session[sid] = sess
	}

	if r.Method == "POST" {
		//update session
		if r.Form.Get("username") == usr && r.Form.Get("password") == pwd {
			sess.Usr = r.Form.Get("username")
			sess.Fname = r.Form.Get("firstname")
			sess.Lname = r.Form.Get("lastname")
		} else {
			draw.Err = "Invalid ID or password"
		}
	}

	t, _ := template.New("05_session").Parse(tpl)
	draw.Sess = *sess
	t.Execute(w, draw)
}

func sess_detory(w http.ResponseWriter, sid string) (sess *Session, err error) {
	var ok bool
	if sess, ok = global_session[sid]; ok {
		delete(global_session, sid)
		cookie := http.Cookie{Name: sess_key, MaxAge: -1}
		http.SetCookie(w, &cookie)
	} else {
		sess = &Session{}
		err = fmt.Errorf("%s not find", sid)
	}
	return
}

func logout(w http.ResponseWriter, r *http.Request) {
	var (
		draw Draw
	)
	draw.Method = r.Method
	//log.Println("logout", r.Method)

	sid, _ := sess_start(w, r)
	sess, _ := sess_detory(w, sid)
	t, _ := template.New("05_session").Parse(tpl)
	draw.Sess = *sess
	fmt.Println(draw)
	t.Execute(w, draw)
}

func handle_null(w http.ResponseWriter, r *http.Request) {
}

func main() {
	log.SetFlags(log.Llongfile)
	http.HandleFunc("/", login)
	http.HandleFunc("/logout", logout)
	http.HandleFunc("/favicon.ico", handle_null)
	err := http.ListenAndServe(":9000", nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}
