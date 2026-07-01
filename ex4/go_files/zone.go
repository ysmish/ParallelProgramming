// 330829847 - Ido Maimon
// 216764803 - Yuli Smishkis
package main

import (
	"math/rand"
	"sync"
	"time"
)

// zoneWorker consumes orders routed to a single delivery zone and processes
// them concurrently, bounded by the zone's token channel. A token is acquired
// before a delivery STARTS and released after it COMPLETES, which guarantees
//     #{orders STARTED but not COMPLETED in this zone} <= cap(tokens).
//
// The per-zone deterministic delay RNG is read only from this single loop
// (before each delivery goroutine is launched), so there is no data race on it.
func zoneWorker(z int, in <-chan Order, tokens chan struct{}, events chan<- Event, seedB int64, wg *sync.WaitGroup) {
	defer wg.Done()

	// Deterministic RNG per zone, exactly as required.
	rng := rand.New(rand.NewSource(seedB + int64(z)*2000003))

	var inFlight sync.WaitGroup
	for order := range in {
		tokens <- struct{}{}      // acquire a token (blocks while the zone is at capacity)
		delayMs := rng.Intn(21)   // draw the delay sequentially => race-free & reproducible call

		inFlight.Add(1)
		go func(o Order, d int) {
			defer inFlight.Done()

			events <- Event{Kind: "STARTED", OrderID: o.OrderID, Zone: z}
			time.Sleep(time.Duration(d) * time.Millisecond)
			events <- Event{Kind: "COMPLETED", OrderID: o.OrderID, Zone: z}

			<-tokens // release the token
		}(order, delayMs)
	}

	inFlight.Wait() // wait for all in-flight deliveries in this zone before signalling done
}