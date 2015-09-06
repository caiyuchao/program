package main

import (
	"encoding/json"
	"log"
	"net/http"
	"time"
)

func demoHandle(w http.ResponseWriter, r *http.Request) {
	js, err := json.Marshal(r)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-type", "application/json; charset=UTF-8")
	w.Write(js)
}

func initRoutes() {
	http.HandleFunc("/demo", demoHandle)
}

func main() {
	initRoutes()
	s := &http.Server{
		Addr:           ":8080",
		ReadTimeout:    10 * time.Second,
		WriteTimeout:   10 * time.Second,
		MaxHeaderBytes: 1 << 20,
	}
	log.Fatal(s.ListenAndServe())
}
