package main

import (
	"database/sql"

	_ "github.com/go-sql-driver/mysql"
)

const dsn string = "root:12341234@tcp(127.0.0.1:3306)/test"

func main() {
	db, err := sql.Open("mysql", dsn)
	if err != nil {
		panic(err.Error()) // Just for example purpose. You should use proper error handling instead of panic
	}
	defer db.Close()

	// Open doesn't open a connection. Validate DSN data:
	err = db.Ping()
	if err != nil {
		panic(err.Error()) // proper error handling instead of panic in your app
	}
}
