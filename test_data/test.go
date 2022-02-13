package main

import (
	"fmt"
	"time"
)

type person struct {
	name string
	age  int
}

type response2 struct {
	Page   int      `json:"page"`
	Fruits []string `json:"fruits"`
}

func newPerson(name string) *person {
	p := person{name: name}
	p.age = 42
	return &p
}

func f(from string) {
	for i := 0; i < 3; i++ {
		fmt.Println(from, ":", i)
	}
}

func main() {
	f("direct")

	go f("goroutine")

	go func(msg string) {
		fmt.Println(msg)
	}("going")

	byt := []byte(`{"num":6.13,"strs":["a","b"]}`)

	time.Sleep(time.Second)
	fmt.Println("done")
}
