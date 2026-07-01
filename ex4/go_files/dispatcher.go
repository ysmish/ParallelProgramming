// 330829847 Ido Maimon
// 216764803 Yuli Smishkis
package main

// dispatch performs fan-in (reading from the single orders stream) and
// fan-out (routing each order to the delivery zone that matches its FoodType).
// It emits a DISPATCHED event for every order. It returns once the orders
// stream has been closed and fully drained. It does NOT close the zone
// channels: those are owned (created and closed) by main.
func dispatch(orders <-chan Order, zoneChans []chan Order, events chan<- Event) {
	for order := range orders {
		events <- Event{Kind: "DISPATCHED", OrderID: order.OrderID, Zone: order.FoodType}
		zoneChans[order.FoodType] <- order
	}
}