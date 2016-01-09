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
	w.Write([]byte("dual_verify\nThis is an example server.\n"))
}

func main() {
	server := flag.Bool("s", false, "server mode")
	server_cert := flag.String("server_cert", "./ca/server.crt", "server A PEM eoncoded certificate file.")
	server_key := flag.String("server_key", "./ca/server.key", "server A PEM encoded private key file.")
	client_cert := flag.String("client_cert", "./ca/client.crt", "client A PEM eoncoded certificate file.")
	client_key := flag.String("client_key", "./ca/client.key", "client A PEM encoded private key file.")
	caFile := flag.String("CA", "./ca/ca.crt", "A PEM eoncoded CA's certificate file.")
	flag.Parse()

	// Load CA cert
	caCert, err := ioutil.ReadFile(*caFile)
	if err != nil {
		log.Fatal(err)
	}
	caCertPool := x509.NewCertPool()
	caCertPool.AppendCertsFromPEM(caCert)

	if *server {
		log.Printf("About to listen %s\n", port)
		http.HandleFunc("/hello", handler)
		serv := &http.Server{
			Addr: ":" + port,
			TLSConfig: &tls.Config{
				ClientCAs:  caCertPool,
				ClientAuth: tls.RequireAndVerifyClientCert,
			},
		}
		err := serv.ListenAndServeTLS(*server_cert, *server_key)
		if err != nil {
			log.Fatal(err)
		}
	} else {
		// Setup HTTPS client
		cliCert, err := tls.LoadX509KeyPair(*client_cert, *client_key)
		if err != nil {
			fmt.Println("Loadx509keypair err:", err)
			return
		}
		client := &http.Client{
			Transport: &http.Transport{
				TLSClientConfig: &tls.Config{
					RootCAs:      caCertPool,
					Certificates: []tls.Certificate{cliCert},
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
