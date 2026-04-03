package handler

import (
	"embermud/internal/game"
	"log"
)

// CharFromRoom removes a character from their current room.
func CharFromRoom(ch *game.CharData) {
	if ch.InRoom == nil {
		log.Println("char_from_room: NULL room")
		return
	}
	ch.InRoom.RemovePerson(ch)
	ch.InRoom = nil
}

// CharToRoom places a character into a room.
func CharToRoom(ch *game.CharData, room *game.RoomIndex, w *game.World) {
	if room == nil {
		log.Println("char_to_room: NULL room")
		room = w.GetRoomIndex(game.RoomVnumTemple)
		if room == nil {
			log.Println("char_to_room: ROOM_VNUM_TEMPLE does not exist")
			return
		}
	}

	ch.InRoom = room
	room.AddPerson(ch)
}

// ExtractChar removes a character from the game.
// If fPull is true, removes from char_list and player_list.
func ExtractChar(ch *game.CharData, fPull bool, w *game.World) {
	if ch == nil {
		return
	}

	if ch.InRoom != nil {
		CharFromRoom(ch)
	}

	if !fPull {
		return
	}

	w.RemoveCharacter(ch)

	if !ch.IsNPC() {
		w.RemovePlayer(ch)
	}

	if ch.Desc != nil {
		ch.Desc.SetCharacter(nil)
	}

	ch.Desc = nil
}

// StopIdling moves an idle character back from limbo.
func StopIdling(ch *game.CharData, w *game.World) {
	if ch == nil || ch.Desc == nil {
		return
	}
	if ch.Desc.GetConnected() != game.ConPlaying {
		return
	}
	if ch.WasInRoom == nil {
		return
	}
	limbo := w.GetRoomIndex(game.RoomVnumLimbo)
	if ch.InRoom != limbo {
		return
	}

	ch.Timer = 0
	CharFromRoom(ch)
	CharToRoom(ch, ch.WasInRoom, w)
	ch.WasInRoom = nil
	game.Act("$n has returned from the void.", ch, nil, nil, game.ToRoom)
}
