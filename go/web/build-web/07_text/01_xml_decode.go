package main

import (
	"encoding/xml"
	"fmt"
	"io/ioutil"
	"os"
)

type Recurlyservers struct {
	XMLName     xml.Name `xml:"servers"`
	Version     string   `xml:"version,attr"`
	Svs         []server `xml:"server"`
	Description string   `xml:",innerxml"`
}

type server struct {
	XMLName    xml.Name `xml:"server"`
	ServerName string   `xml:"serverName"`
	ServerIP   string   `xml:"serverIP"`
}

func main() {
	file, err := os.Open("servers.xml") // For read access.
	if err != nil {
		fmt.Printf("error: %v", err)
		return
	}
	defer file.Close()
	data, err := ioutil.ReadAll(file)
	if err != nil {
		fmt.Printf("error: %v", err)
		return
	}
	v := Recurlyservers{}
	err = xml.Unmarshal(data, &v)
	if err != nil {
		fmt.Printf("error: %v", err)
		return
	}
	fmt.Println("xmlName:", v.XMLName)
	fmt.Println("version:", v.Version)
	for k1, v1 := range v.Svs {
		fmt.Println(k1, "xmlname.local:", v1.XMLName.Local)
		fmt.Println(k1, "xmlname.space:", v1.XMLName.Space)
		fmt.Println(k1, "servername:", v1.ServerName)
		fmt.Println(k1, "serverIP:", v1.ServerIP)
	}
	fmt.Println("Description:", v.Description)

}
