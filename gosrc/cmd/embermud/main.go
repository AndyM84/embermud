package main

import (
	"embermud/internal/commands"
	"embermud/internal/game"
	"embermud/internal/handler"
	"embermud/internal/network"
	"embermud/internal/persist"
	"embermud/internal/update"
	"flag"
	"log"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"
)

func main() {
	port := flag.Int("port", 9000, "Port to listen on")
	flag.Parse()

	if *port <= 1024 {
		log.Fatal("Port number must be above 1024.")
	}

	// Ensure player directory exists
	os.MkdirAll(game.CfgPlayerDir, 0755)

	// Create the world
	w := game.NewWorld()

	// Wire up area loading
	game.AreaLoader = func(w *game.World) {
		persist.LoadAreaFiles(w)
		// Fix exits after loading
		if w.GetRoomIndex(game.RoomVnumLimbo) != nil || w.GetRoomIndex(game.RoomVnumTemple) != nil {
			persist.FixExits(w)
		}
	}

	// Boot the world (loads areas or creates minimal world)
	w.Boot()

	// Create command table
	cmdTable := commands.NewCommandTable(w)

	// Wire save function into commands package
	saveFn := func(ch *game.CharData) {
		persist.SaveCharObj(ch, w)
	}
	commands.SaveFn = saveFn

	// Wire load function for nanny
	loadFn := func(d *network.Desc, name string) bool {
		return persist.LoadCharObj(d, name, w)
	}

	// Create update handler
	updater := update.NewUpdateHandler(w, saveFn, func(ch *game.CharData, arg string) {
		cmdTable.Interpret(ch, "quit")
	})

	// Start TCP listener
	newConnCh := make(chan *network.Desc, 32)
	err := network.ListenAndServe(*port, w, newConnCh)
	if err != nil {
		log.Fatalf("Failed to start server: %v", err)
	}

	// Handle signals
	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, syscall.SIGINT, syscall.SIGTERM)

	// Main game loop
	ticker := time.NewTicker(game.PulseDuration)
	defer ticker.Stop()

	log.Println("Entering game loop.")

	for !w.MercDown {
		select {
		case <-sigCh:
			log.Println("Received shutdown signal.")
			w.MercDown = true
			continue

		case <-ticker.C:
			w.CurrentTime = time.Now()

			// Accept new connections (non-blocking drain)
			drainNewConns(newConnCh)

			// Process input from all descriptors
			processInput(w, cmdTable, saveFn, loadFn)

			// Update game state
			updater.Pulse()

			// Process output for all descriptors
			processOutput(w)
		}
	}

	// Shutdown: save all players and close connections
	for _, d := range w.Descriptors {
		desc, ok := d.(*network.Desc)
		if !ok {
			continue
		}
		if desc.GetCharacter() != nil && desc.GetCharacter().Level > 1 {
			persist.SaveCharObj(desc.GetCharacter(), w)
		}
		desc.Close()
	}

	log.Println("Normal termination of game.")
}

func drainNewConns(ch <-chan *network.Desc) {
	for {
		select {
		case <-ch:
			// Connection already added to world by server.go
		default:
			return
		}
	}
}

func processInput(w *game.World, cmdTable *commands.CommandTable,
	saveFn func(*game.CharData), loadFn func(*network.Desc, string) bool) {

	// Copy descriptor list since it may be modified during processing
	descs := make([]game.Descriptor, len(w.Descriptors))
	copy(descs, w.Descriptors)

	for _, d := range descs {
		desc, ok := d.(*network.Desc)
		if !ok {
			continue
		}

		desc.FCommand = false

		// Non-blocking read of one command from input channel
		var input string
		select {
		case line, ok := <-desc.InputCh:
			if !ok {
				// Connection closed
				if desc.GetCharacter() != nil && desc.GetCharacter().Level > 1 {
					persist.SaveCharObj(desc.GetCharacter(), w)
				}
				desc.Close()
				continue
			}
			input = line
		default:
			// No input available
		}

		// Respect wait state
		ch := desc.GetCharacter()
		if ch != nil && ch.Wait > 0 {
			ch.Wait--
			continue
		}

		if input == "" {
			continue
		}

		// Blank input (just whitespace / telnet noise) should only be
		// processed when the pager is active or we're in the nanny
		// login flow. During normal gameplay it would just spam prompts.
		isBlank := strings.TrimSpace(input) == ""
		hasPager := desc.ShowStrHead != "" && desc.ShowStrPoint < len(desc.ShowStrHead)
		isPlaying := desc.GetConnected() == game.ConPlaying

		if isBlank && isPlaying && !hasPager {
			continue
		}

		desc.FCommand = true

		if ch != nil {
			ch.Timer = 0
			handler.StopIdling(ch, w)
		}

		if hasPager {
			desc.ShowString(input)
		} else if isPlaying {
			network.SubstituteAlias(desc, input, cmdTable.Interpret)
		} else {
			network.Nanny(desc, input, cmdTable.Interpret, saveFn, loadFn)
		}
	}
}

func processOutput(w *game.World) {
	descs := make([]game.Descriptor, len(w.Descriptors))
	copy(descs, w.Descriptors)

	for _, d := range descs {
		desc, ok := d.(*network.Desc)
		if !ok {
			continue
		}
		// Only process output when there's a command or buffered output,
		// matching the C code: (d->fcommand || d->outtop > 0)
		if desc.FCommand || desc.HasBufferedOutput() {
			desc.ProcessOutput()
		}
	}
}
