// 330829847 - Ido Maimon
// 216764803 - Yuli Smishkis
package main

import "fmt"

// logger is the ONLY goroutine allowed to print to stdout. It drains the
// events channel and prints each event in the required strict format.
func logger(events <-chan Event) {
	for e := range events {
		switch e.Kind {
		case "CREATED":
			fmt.Printf("CREATED order=%d restaurant=%d type=%d\n", e.OrderID, e.RestaurantID, e.Zone)
		case "DISPATCHED":
			fmt.Printf("DISPATCHED order=%d zone=%d\n", e.OrderID, e.Zone)
		case "STARTED":
			fmt.Printf("STARTED order=%d zone=%d\n", e.OrderID, e.Zone)
		case "COMPLETED":
			fmt.Printf("COMPLETED order=%d zone=%d\n", e.OrderID, e.Zone)
		case "DONE":
			fmt.Printf("DONE total=%d\n", e.Total)
		}
	}
}