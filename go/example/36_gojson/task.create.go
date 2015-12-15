package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
)

type Foo struct {
	Area   string `json:"area"`
	End    int    `json:"end"`
	Host   string `json:"host"`
	IP     string `json:"ip"`
	Isp    string `json:"isp"`
	Metric string `json:"metric"`
	Start  int    `json:"start"`
	Step   int    `json:"step"`
}

func main() {
	var foo []Foo
	ctx, _ := ioutil.ReadFile("task.create.json")
	json.Unmarshal([]byte(string(ctx)), &foo)
	fmt.Println(foo)

}
