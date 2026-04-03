package network

import (
	"embermud/internal/game"
	"fmt"
	"log"
	"net"
)

// ListenAndServe starts the TCP listener and sends new connections to newConnCh.
func ListenAndServe(port int, world *game.World, newConnCh chan<- *Desc) error {
	addr := fmt.Sprintf(":%d", port)
	listener, err := net.Listen("tcp", addr)
	if err != nil {
		return fmt.Errorf("init_socket: %w", err)
	}

	log.Printf("EmberMUD is ready to rock on port %d.", port)

	go func() {
		defer listener.Close()
		for {
			conn, err := listener.Accept()
			if err != nil {
				if world.MercDown {
					return
				}
				log.Printf("accept error: %v", err)
				continue
			}

			desc := NewDesc(conn, world)

			// Check bans
			if game.CheckBan(desc.Host, world.Bans) {
				desc.WriteToBuffer("Your site has been banned.\n\r")
				desc.FlushOutput()
				conn.Close()
				continue
			}

			desc.StartIO()

			// Send welcome message and ANSI prompt
			desc.WriteToBuffer(game.CfgConnectMsg)
			desc.WriteToBuffer(game.CfgAskAnsi)
			desc.FlushOutput()

			world.AddDescriptor(desc)

			select {
			case newConnCh <- desc:
			default:
				log.Printf("Warning: newConnCh full, dropping connection from %s", desc.Host)
				conn.Close()
			}
		}
	}()

	return nil
}
