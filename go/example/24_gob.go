// Copyright 2013 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"bytes"
	"encoding/gob"
	"fmt"
	"log"
)

type P struct {
	X int
}

type Q struct {
	X *int32
}

// This example shows the basic usage of the package: Create an encoder,
// transmit some values, receive them with a decoder.
func main() {
	// Initialize the encoder and decoder.  Normally enc and dec would be
	// bound to network connections and the encoder and decoder would
	// run in different processes.
	var network bytes.Buffer        // Stand-in for a network connection
	enc := gob.NewEncoder(&network) // Will write to network.
	dec := gob.NewDecoder(&network) // Will read from network.

	// Encode (send) some values.
	err := enc.Encode(P{3})
	if err != nil {
		log.Fatal("encode error:", err)
	}
	log.Printf("%x\n", network)
	err = enc.Encode(P{1782})
	if err != nil {
		log.Fatal("encode error:", err)
	}
	log.Printf("%x\n", network)

	// Decode (receive) and print the values.
	var q Q
	err = dec.Decode(&q)
	if err != nil {
		log.Fatal("decode error 1:", err)
	}
	fmt.Printf("%d\n", *q.X)
	log.Printf("%x\n", network)
	err = dec.Decode(&q)
	if err != nil {
		log.Fatal("decode error 2:", err)
	}
	fmt.Printf("%d\n", *q.X)
	log.Printf("%x\n", network)

	// Output:
	// "Pythagoras": {3, 4}
	// "Treehouse": {1782, 1841}
}
