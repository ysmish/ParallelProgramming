// 330829847 Ido Maimon
// 216764803 Yuli Smishkis
package main

// Order is a single food order produced by a restaurant.
type Order struct {
	OrderID      int // unique in [0..N-1]
	RestaurantID int // in [0..R-1]
	FoodType     int // zone index in [0..Z-1]
}

// Event represents a system event. Events are used only for printing
// the required output format (handled by the single logger goroutine).
type Event struct {
	Kind         string // "CREATED" | "DISPATCHED" | "STARTED" | "COMPLETED" | "DONE"
	OrderID      int
	RestaurantID int
	Zone         int
	Total        int // used only by the DONE event
}