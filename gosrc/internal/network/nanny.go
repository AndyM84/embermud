package network

import (
	"embermud/internal/game"
	"fmt"
	"log"
	"strings"
	"unicode"

	"golang.org/x/crypto/bcrypt"
)

// NannyFunc is the type for the command interpreter callback.
type NannyFunc func(ch *game.CharData, argument string)

// SaveFunc is the type for saving a character.
type SaveFunc func(ch *game.CharData)

// LoadFunc is the type for loading a character. Returns true if existing player found.
type LoadFunc func(d *Desc, name string) bool

// Nanny handles login state machine for connections not yet playing.
func Nanny(d *Desc, argument string, interpretFn NannyFunc, saveFn SaveFunc, loadFn LoadFunc) {
	argument = strings.TrimSpace(argument)
	ch := d.character
	w := d.World

	switch d.connected {

	case game.ConGetAnsi:
		if len(argument) > 0 && (argument[0] == 'n' || argument[0] == 'N') {
			d.ansi = false
		} else {
			d.ansi = true
		}

		greeting := w.HelpGreeting
		if len(greeting) > 0 && greeting[0] == '.' {
			d.WriteToBuffer(greeting[1:])
		} else {
			d.WriteToBuffer(greeting)
		}

		d.connected = game.ConGetName

	case game.ConGetName:
		if argument == "" {
			d.Close()
			return
		}

		// Capitalize first letter
		runes := []rune(argument)
		runes[0] = unicode.ToUpper(runes[0])
		argument = string(runes)

		if !game.CheckParseName(argument) {
			d.WriteToBuffer("Illegal name, try another.\n\rName: ")
			return
		}

		fOld := loadFn(d, argument)
		ch = d.character

		if game.IsSet(ch.Act, game.PlrDeny) {
			log.Printf("Denying access to %s@%s.", argument, d.Host)
			d.WriteToBuffer("You are denied access.\n\r")
			d.Close()
			return
		}

		if checkReconnect(d, argument, false) {
			fOld = true
		} else if w.Wizlock && !ch.IsHero() {
			d.WriteToBuffer("The game is wizlocked.\n\r")
			d.Close()
			return
		}

		if fOld {
			d.WriteToBuffer("Password: ")
			d.WriteToBuffer(EchoOffStr)
			d.connected = game.ConGetOldPassword
		} else {
			if w.Newlock {
				d.WriteToBuffer("The game is newlocked.\n\r")
				d.Close()
				return
			}
			d.WriteToBuffer(fmt.Sprintf("Did I get that right, %s (Y/N)? ", argument))
			d.connected = game.ConConfirmNewName
		}

	case game.ConGetOldPassword:
		d.WriteToBuffer("\n\r")

		if ch.PCData == nil || !checkPassword(argument, ch.PCData.Password) {
			d.WriteToBuffer("Wrong password.\n\r")
			d.Close()
			return
		}

		d.WriteToBuffer(EchoOnStr)

		if checkReconnect(d, ch.Name, true) {
			return
		}
		if checkPlaying(d, ch.Name) {
			return
		}

		log.Printf("%s@%s has connected.", ch.Name, d.Host)
		doHelp(ch, "motd", w)
		d.connected = game.ConReadMotd

	case game.ConBreakConnect:
		switch {
		case len(argument) > 0 && (argument[0] == 'y' || argument[0] == 'Y'):
			// Find and close old descriptor
			for _, od := range w.Descriptors {
				oldDesc, ok := od.(*Desc)
				if !ok || oldDesc == d || oldDesc.character == nil {
					continue
				}
				if !strings.EqualFold(ch.Name, oldDesc.character.Name) {
					continue
				}
				oldDesc.Close()
				break
			}
			d.WriteToBuffer("Disconnected.   Re-enter name: ")
			d.character = nil
			d.connected = game.ConGetName

		case len(argument) > 0 && (argument[0] == 'n' || argument[0] == 'N'):
			d.WriteToBuffer("Name: ")
			d.character = nil
			d.connected = game.ConGetName

		default:
			d.WriteToBuffer("Please type Y or N? ")
		}

	case game.ConConfirmNewName:
		switch {
		case len(argument) > 0 && (argument[0] == 'y' || argument[0] == 'Y'):
			d.WriteToBuffer(fmt.Sprintf("New character.\n\rGive me a password for %s: %s",
				ch.Name, EchoOffStr))
			d.connected = game.ConGetNewPassword
			if d.ansi {
				game.SetBit(&ch.Act, game.PlrColour)
			}

		case len(argument) > 0 && (argument[0] == 'n' || argument[0] == 'N'):
			d.WriteToBuffer("Ok, what IS it, then? ")
			d.character = nil
			d.connected = game.ConGetName

		default:
			d.WriteToBuffer("Please type Yes or No? ")
		}

	case game.ConGetNewPassword:
		d.WriteToBuffer("\n\r")
		if len(argument) < 5 {
			d.WriteToBuffer("Password must be at least five characters long.\n\rPassword: ")
			return
		}

		hashed, err := bcrypt.GenerateFromPassword([]byte(argument), bcrypt.DefaultCost)
		if err != nil {
			d.WriteToBuffer("New password not acceptable, try again.\n\rPassword: ")
			return
		}

		ch.PCData.Password = string(hashed)
		d.WriteToBuffer("Please retype password: ")
		d.connected = game.ConConfirmNewPassword

	case game.ConConfirmNewPassword:
		d.WriteToBuffer("\n\r")
		if !checkPassword(argument, ch.PCData.Password) {
			d.WriteToBuffer("Passwords don't match.\n\rRetype password: ")
			d.connected = game.ConGetNewPassword
			return
		}

		d.WriteToBuffer(EchoOnStr)
		d.WriteToBuffer("\n\rThe following roles are available:\n\r")
		d.WriteToBuffer("  [1] Wanderer  - A traveler and explorer\n\r")
		d.WriteToBuffer("  [2] Merchant  - A trader and craftsperson\n\r")
		d.WriteToBuffer("  [3] Scholar   - A seeker of knowledge\n\r")
		d.WriteToBuffer("  [4] Guardian  - A protector of the realm\n\r")
		d.WriteToBuffer("\n\rSelect a role: ")
		d.connected = game.ConGetNewRole

	case game.ConGetNewRole:
		if len(argument) == 0 {
			d.WriteToBuffer("That's not a valid role.\n\rSelect a role [1-4]: ")
			return
		}
		switch argument[0] {
		case '1':
			setTitle(ch, " the Wanderer")
		case '2':
			setTitle(ch, " the Merchant")
		case '3':
			setTitle(ch, " the Scholar")
		case '4':
			setTitle(ch, " the Guardian")
		default:
			d.WriteToBuffer("That's not a valid role.\n\rSelect a role [1-4]: ")
			return
		}

		log.Printf("%s@%s new player.", ch.Name, d.Host)
		d.WriteToBuffer("\n\r")
		doHelp(ch, "motd", w)
		d.connected = game.ConReadMotd

	case game.ConReadMotd:
		d.WriteToBuffer("\n\rWelcome to EmberMUD.\n\r")

		w.AddCharacter(ch)
		w.AddPlayer(ch)

		d.connected = game.ConPlaying

		// Normalize title
		if ch.PCData != nil && ch.PCData.Title != "" {
			setTitle(ch, ch.PCData.Title)
		}

		if ch.Level == 0 {
			ch.Level = 1
			room := w.GetRoomIndex(game.RoomVnumTemple)
			if room != nil {
				charToRoom(ch, room)
			}
			ch.SendToChar("\n\r")
			saveFn(ch)
		} else if ch.InRoom != nil {
			charToRoom(ch, ch.InRoom)
		} else {
			room := w.GetRoomIndex(game.RoomVnumTemple)
			if room != nil {
				charToRoom(ch, room)
			}
		}

		game.Act("$n has entered the game.", ch, nil, nil, game.ToRoom)
		interpretFn(ch, "look auto")
		saveFn(ch)

	default:
		log.Printf("Nanny: bad connected state %d", d.connected)
		d.Close()
	}
}

// checkPassword compares a plaintext password against a bcrypt hash.
func checkPassword(plain, hashed string) bool {
	if hashed == "" {
		return plain == ""
	}
	// Try bcrypt first
	err := bcrypt.CompareHashAndPassword([]byte(hashed), []byte(plain))
	if err == nil {
		return true
	}
	// Fall back to plaintext comparison for old C save files
	return plain == hashed
}

// checkReconnect looks for a link-dead player to reconnect.
func checkReconnect(d *Desc, name string, fConn bool) bool {
	w := d.World
	for _, ch := range w.Players {
		if (!fConn || ch.Desc == nil) && strings.EqualFold(d.character.Name, ch.Name) {
			if !fConn {
				// Just copy the password
				if d.character.PCData != nil && ch.PCData != nil {
					d.character.PCData.Password = ch.PCData.Password
				}
			} else {
				// Full reconnect
				d.character = ch
				ch.Desc = d
				ch.Timer = 0
				ch.SendToChar("Reconnecting.\n\r")
				game.Act("$n has reconnected.", ch, nil, nil, game.ToRoom)
				log.Printf("%s@%s reconnected.", ch.Name, d.Host)
				d.connected = game.ConPlaying
			}
			return true
		}
	}
	return false
}

// checkPlaying checks if the character is already playing.
func checkPlaying(d *Desc, name string) bool {
	w := d.World
	for _, od := range w.Descriptors {
		dold, ok := od.(*Desc)
		if !ok || dold == d || dold.character == nil {
			continue
		}
		if dold.connected == game.ConGetName || dold.connected == game.ConGetOldPassword {
			continue
		}
		if strings.EqualFold(name, dold.character.Name) {
			d.WriteToBuffer("That character is already playing.\n\rDisconnect that player? ")
			d.connected = game.ConBreakConnect
			return true
		}
	}
	return false
}

// setTitle sets a player's title with leading space.
func setTitle(ch *game.CharData, title string) {
	if ch.IsNPC() || ch.PCData == nil {
		return
	}
	if len(title) > 0 && title[0] != ' ' {
		title = " " + title
	}
	ch.PCData.Title = title
}

// doHelp displays a help topic to a character.
func doHelp(ch *game.CharData, keyword string, w *game.World) {
	h := w.FindHelp(ch, keyword)
	if h == nil {
		return
	}
	text := h.Text
	if len(text) > 0 && text[0] == '.' {
		text = text[1:]
	}
	ch.SendToChar(text)
}

// charToRoom places a character into a room (used during nanny only).
func charToRoom(ch *game.CharData, room *game.RoomIndex) {
	if room == nil {
		return
	}
	ch.InRoom = room
	room.AddPerson(ch)
}
