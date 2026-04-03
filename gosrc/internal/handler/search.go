package handler

import (
	"embermud/internal/game"
)

// GetCharRoom finds a character in the same room by name.
func GetCharRoom(ch *game.CharData, argument string) *game.CharData {
	if ch == nil || ch.InRoom == nil {
		return nil
	}

	number, arg := game.NumberArgument(argument)
	if arg == "" {
		return nil
	}

	count := 0
	for _, rch := range ch.InRoom.People {
		if !CanSee(ch, rch) {
			continue
		}
		if !game.IsName(arg, rch.Name) {
			continue
		}
		count++
		if count == number {
			return rch
		}
	}
	return nil
}

// GetPlayerWorld finds a player anywhere in the world by name.
func GetPlayerWorld(ch *game.CharData, argument string, w *game.World) *game.CharData {
	number, arg := game.NumberArgument(argument)
	if arg == "" {
		return nil
	}

	count := 0
	for _, wch := range w.Players {
		if !CanSee(ch, wch) {
			continue
		}
		if !game.IsName(arg, wch.Name) {
			continue
		}
		count++
		if count == number {
			return wch
		}
	}
	return nil
}

// CanSee is a simplified visibility check.
func CanSee(ch, victim *game.CharData) bool {
	if ch == victim {
		return true
	}
	if victim.InvisLevel > ch.GetTrust() {
		return false
	}
	return true
}

// CanSeeRoom always returns true in this stripped core.
func CanSeeRoom(ch *game.CharData, room *game.RoomIndex) bool {
	return true
}

// RoomIsDark checks if a room is dark.
func RoomIsDark(room *game.RoomIndex) bool {
	if room == nil {
		return false
	}
	return game.IsSet(room.RoomFlags, game.RoomDark) && room.Light < 1
}

// RoomIsPrivate checks room occupancy limits.
func RoomIsPrivate(room *game.RoomIndex) bool {
	if room == nil {
		return false
	}

	count := len(room.People)

	if game.IsSet(room.RoomFlags, game.RoomPrivate) && count >= 2 {
		return true
	}
	if game.IsSet(room.RoomFlags, game.RoomSolitary) && count >= 1 {
		return true
	}
	return false
}
