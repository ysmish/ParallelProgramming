// 330829847 Ido Maimon
// 216764803 Yuli Smishkis
package main

import (
	"math/rand"
	"sync"
)

// restaurant is a producer goroutine. Restaurant rid generates exactly the
// contiguous OrderID range [rid*N/R, (rid+1)*N/R), chooses a zone for each
// order using a per-restaurant deterministic RNG, emits a CREATED event, and
// sends the order into the shared fan-in stream.
func restaurant(rid, n, r, z int, seedA int64, orders chan<- Order, events chan<- Event, wg *sync.WaitGroup) {
	defer wg.Done()

	startID := rid * n / r
	endID := (rid + 1) * n / r

	// Deterministic RNG per restaurant, exactly as required.
	rng := rand.New(rand.NewSource(seedA + int64(rid)*1000003))

	for id := startID; id < endID; id++ {
		zone := rng.Intn(z)
		order := Order{OrderID: id, RestaurantID: rid, FoodType: zone}

		// CREATED is emitted before the order is dispatched.
		events <- Event{Kind: "CREATED", OrderID: id, RestaurantID: rid, Zone: zone}
		orders <- order
	}
}