/*
 * ./key.sh
 * go run xxx.go -s true  // server mode
 * go run xxx.go          // client mode
 * yubo@yubo.org
 */
package main

import (
	"crypto/tls"
	"crypto/x509"

	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
)

const (
	port = "12348"
)

func handler(w http.ResponseWriter, req *http.Request) {
	w.Header().Set("Content-Type", "text/plain")
	w.Write([]byte("verify_server\nThis is an example server.\n"))
}

func main() {
	server := flag.Bool("s", false, "server mode")
	certFile := flag.String("cert", "./ca/server.crt", "A PEM eoncoded certificate file.")
	keyFile := flag.String("key", "./ca/server.key", "A PEM encoded private key file.")
	caFile := flag.String("CA", "./ca/ca.crt", "A PEM eoncoded CA's certificate file.")
	flag.Parse()

	if *server {
		http.HandleFunc("/hello", handler)
		log.Printf("About to listen %s\n", port)
		err := http.ListenAndServeTLS(":"+port, *certFile, *keyFile, nil)
		if err != nil {
			log.Fatal(err)
		}
	} else {
		// Load CA cert
		caCert, err := ioutil.ReadFile(*caFile)
		if err != nil {
			log.Fatal(err)
		}
		caCertPool := x509.NewCertPool()
		caCertPool.AppendCertsFromPEM(caCert)

		// Setup HTTPS client
		client := &http.Client{
			Transport: &http.Transport{
				TLSClientConfig: &tls.Config{
					RootCAs: caCertPool,
				},
			},
		}

		// Do GET something
		resp, err := client.Get(fmt.Sprintf("https://test.yubo.org:%s/hello", port))
		if err != nil {
			log.Fatal(err)
		}
		defer resp.Body.Close()

		// Dump response
		data, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println(string(data))
	}
}
