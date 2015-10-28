package main

import (
	"fmt"
	"html/template"
	"os"
	"strings"
)

type Person struct {
	UserName string
	Email    string
}

func insert_file() {
	t, _ := template.ParseFiles("./test.tpl")
	p := Person{UserName: "admin", Email: "admin@example.com"}
	t.Execute(os.Stdout, p)
}

func insert_str() {
	t := template.New("fieldname example")
	t, _ = t.Parse("hello {{.UserName}}! {{.Email}}\n")
	p := Person{UserName: "admin", Email: "admin@example.com"}
	t.Execute(os.Stdout, p)
}

func EmailDealWith(args ...interface{}) string {
	ok := false
	var s string
	if len(args) == 1 {
		s, ok = args[0].(string)
	}
	if !ok {
		s = fmt.Sprint(args...)
	}
	// find the @ symbol
	substrs := strings.Split(s, "@")
	if len(substrs) != 2 {
		return s
	}
	// replace the @ by " at "
	return (substrs[0] + " at " + substrs[1])
}

func nested() {
	type Friend struct {
		Fname string
	}

	type Person struct {
		UserName string
		Emails   []string
		Friends  []*Friend
	}
	f1 := Friend{Fname: "minux.ma"}
	f2 := Friend{Fname: "xushiwei"}
	t := template.New("fieldname example")
	t = t.Funcs(template.FuncMap{"emailDeal": EmailDealWith})
	t, _ = t.Parse(
		`hello {{.UserName}}!  <!-- 使用range循环 -->
    {{range .Emails}}
        an email {{. | emailDeal}}
    {{end}}
	<!-- 使用witch + range循环 -->
    {{with .Friends}}
    {{range .}}
        my friend name is {{.Fname}}
    {{end}}
    {{end}}
`)
	p := Person{UserName: "Astaxie",
		Emails:  []string{"astaxie@beego.me", "astaxie@gmail.com"},
		Friends: []*Friend{&f1, &f2}}
	t.Execute(os.Stdout, p)
}

func conditions() {
	tEmpty := template.New("template test")
	tEmpty = template.Must(tEmpty.Parse("Empty pipeline if demo: {{if ``}} will not be outputted. {{end}}\n"))
	tEmpty.Execute(os.Stdout, nil)

	tWithValue := template.New("template test")
	tWithValue = template.Must(tWithValue.Parse("Not empty pipeline if demo: {{if `anything`}} will be outputted. {{end}}\n"))
	tWithValue.Execute(os.Stdout, nil)

	tIfElse := template.New("template test")
	tIfElse = template.Must(tIfElse.Parse("if-else demo: {{if `anything`}} if part {{else}} else part.{{end}}\n"))
	tIfElse.Execute(os.Stdout, nil)
}

func pipelines() {
	t := template.New("template test")
	t, _ = t.Parse("{{. | html}}\n")
	t.Execute(os.Stdout, "<script>aaa</script>")
}

func tpl_var() {
	t := template.New("template test")
	t, _ = t.Parse(`{{with $x := "output" | printf "%q"}}{{$x}}{{end}}
{{with $x := "output"}}{{printf "%q" $x}}{{end}}
{{with $x := "output"}}{{$x | printf "%q"}}{{end}}
`)
	t.Execute(os.Stdout, nil)
}

func include_tpl() {
	s1, _ := template.ParseFiles("header.tpl", "content.tpl", "footer.tpl")
	s1.ExecuteTemplate(os.Stdout, "header", nil)
	fmt.Println()
	s1.ExecuteTemplate(os.Stdout, "content", nil)
	fmt.Println()
	s1.ExecuteTemplate(os.Stdout, "footer", nil)
	fmt.Println()
	s1.Execute(os.Stdout, nil)
}

func main() {
	insert_file()
	insert_str()
	nested()
	conditions()
	pipelines()
	tpl_var()
	include_tpl()
}
