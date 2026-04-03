package update

import (
	"embermud/internal/game"
	"embermud/internal/handler"
	"log"
)

// SaveFunc is the save callback.
type SaveFunc func(ch *game.CharData)

// QuitFunc is the quit callback (calls do_quit).
type QuitFunc func(ch *game.CharData, argument string)

// UpdateHandler is called once per pulse from the game loop.
type UpdateHandler struct {
	World      *game.World
	SaveFn     SaveFunc
	QuitFn     QuitFunc
	pulsePoint int
	saveNumber int
}

// NewUpdateHandler creates a new UpdateHandler.
func NewUpdateHandler(w *game.World, saveFn SaveFunc, quitFn QuitFunc) *UpdateHandler {
	return &UpdateHandler{
		World:      w,
		SaveFn:     saveFn,
		QuitFn:     quitFn,
		pulsePoint: game.PulseTick,
	}
}

// Pulse is called once per game pulse (4x per second).
func (u *UpdateHandler) Pulse() {
	u.pulsePoint--
	if u.pulsePoint <= 0 {
		u.pulsePoint = game.PulseTick
		u.charUpdate()
	}
}

func (u *UpdateHandler) charUpdate() {
	w := u.World

	u.saveNumber++
	if u.saveNumber > 30 {
		u.saveNumber = 0
	}

	var chQuit *game.CharData

	// Make a copy of the slice since we may modify it
	chars := make([]*game.CharData, len(w.Characters))
	copy(chars, w.Characters)

	for _, ch := range chars {
		if ch.IsNPC() {
			continue
		}

		// Increment played time
		elapsed := int(w.CurrentTime.Sub(ch.Logon).Seconds())
		if elapsed > 0 {
			ch.Played += elapsed
			ch.Logon = w.CurrentTime
		}

		// Increment idle timer
		ch.Timer++

		// Idle timeout: move to limbo after 12 ticks
		if ch.Timer >= 12 && !ch.IsImmortal() {
			if ch.WasInRoom == nil && ch.InRoom != nil {
				ch.WasInRoom = ch.InRoom
				game.Act("$n disappears into the void.", ch, nil, nil, game.ToRoom)
				ch.SendToChar("You disappear into the void.\n\r")
				if ch.Level > 1 && u.SaveFn != nil {
					u.SaveFn(ch)
				}
				handler.CharFromRoom(ch)
				limbo := w.GetRoomIndex(game.RoomVnumLimbo)
				if limbo != nil {
					handler.CharToRoom(ch, limbo, w)
				}
			}
		}

		// Auto-quit after 30 ticks idle
		if ch.Timer > 30 && !ch.IsImmortal() {
			chQuit = ch
		}
	}

	// Auto-save and auto-quit pass
	players := make([]*game.CharData, len(w.Players))
	copy(players, w.Players)

	for _, ch := range players {
		if ch.Desc != nil && u.saveNumber == 30 && u.SaveFn != nil {
			u.SaveFn(ch)
		}

		if ch == chQuit && u.QuitFn != nil {
			log.Printf("%s auto-quit (idle timeout).", ch.Name)
			u.QuitFn(ch, "")
		}
	}
}
