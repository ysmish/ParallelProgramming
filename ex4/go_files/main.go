// 330829847 - Ido Maimon
// 216764803 - Yuli Smishkis
package main

import (
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"
	"sync"
)

// parseTokens converts the comma-separated --tokens argument into a slice of
// per-zone token counts and validates that its length matches the zone count.
func parseTokens(s string, zones int) ([]int, error) {
	parts := strings.Split(s, ",")
	if len(parts) != zones {
		return nil, fmt.Errorf("expected %d token values, got %d", zones, len(parts))
	}
	out := make([]int, zones)
	for i, p := range parts {
		v, err := strconv.Atoi(strings.TrimSpace(p))
		if err != nil {
			return nil, fmt.Errorf("invalid token value %q: %v", p, err)
		}
		out[i] = v
	}
	return out, nil
}

func main() {
	// ---- Command-line arguments ----
	n := flag.Int("n", 0, "total number of orders")
	restaurants := flag.Int("restaurants", 1, "number of restaurants")
	zones := flag.Int("zones", 1, "number of delivery zones")
	tokensArg := flag.String("tokens", "", "comma-separated concurrent-delivery limit per zone")
	seedA := flag.Int64("seedA", 0, "seed for order generation")
	seedB := flag.Int64("seedB", 0, "seed for processing delays")
	flag.Parse()

	tokenCounts, err := parseTokens(*tokensArg, *zones)
	if err != nil {
		fmt.Fprintln(os.Stderr, "argument error:", err)
		os.Exit(1)
	}

	N, R, Z := *n, *restaurants, *zones

	// ---- Single logger goroutine (only writer to stdout) ----
	events := make(chan Event, 4096)
	var loggerWg sync.WaitGroup
	loggerWg.Add(1)
	go func() {
		defer loggerWg.Done()
		logger(events)
	}()

	// ---- Restaurants (producers) -> shared fan-in stream ----
	orders := make(chan Order, R+1)
	var restWg sync.WaitGroup
	for rid := 0; rid < R; rid++ {
		restWg.Add(1)
		go restaurant(rid, N, R, Z, *seedA, orders, events, &restWg)
	}
	// main owns 'orders', so main closes it once all restaurants are done.
	go func() {
		restWg.Wait()
		close(orders)
	}()

	// ---- Delivery zones (workers) + per-zone token channels ----
	zoneChans := make([]chan Order, Z)
	tokenChans := make([]chan struct{}, Z)
	var zoneWg sync.WaitGroup
	for z := 0; z < Z; z++ {
		capZ := tokenCounts[z]
		if capZ < 1 {
			capZ = 1 // guard: a zone must be able to make progress
		}
		zoneChans[z] = make(chan Order, N+1)
		tokenChans[z] = make(chan struct{}, capZ)
		zoneWg.Add(1)
		go zoneWorker(z, zoneChans[z], tokenChans[z], events, *seedB, &zoneWg)
	}

	// ---- Dispatcher (fan-in / fan-out) runs on the main goroutine ----
	dispatch(orders, zoneChans, events)

	// orders drained -> main (creator) closes the zone channels.
	for z := 0; z < Z; z++ {
		close(zoneChans[z])
	}

	// ---- Clean shutdown ----
	zoneWg.Wait()                                  // all deliveries finished
	events <- Event{Kind: "DONE", Total: N}        // final line, printed by the logger
	close(events)                                  // main owns 'events'
	loggerWg.Wait()
}