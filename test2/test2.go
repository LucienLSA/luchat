package main

import (
	"sync"
)

func main() {
	order := make(chan struct{})
	var wg sync.WaitGroup
	for i := 1; i <= 1000; i++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			<-order
			println(id)
			if id != 1000 {
				order <- struct{}{}
			}
		}(i)
	}
	order <- struct{}{}
	wg.Wait()
	close(order)
}
