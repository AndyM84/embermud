package commands

import (
	"embermud/internal/game"
	"embermud/internal/handler"
	"fmt"
	"log"
	"strings"
	"unicode"
)

// CmdFunc is the type for command handler functions.
type CmdFunc func(ch *game.CharData, argument string)

// CmdEntry represents a command in the command table.
type CmdEntry struct {
	Name     string
	DoFun    CmdFunc
	Position int
	Level    int
	Log      int
}

// CommandTable holds all registered commands.
type CommandTable struct {
	entries []*CmdEntry
	world   *game.World
}

// NewCommandTable creates and populates the command table.
func NewCommandTable(w *game.World) *CommandTable {
	ct := &CommandTable{world: w}

	// Movement commands (priority at top for speed)
	ct.add("north", ct.doNorth, game.PosStanding, 0, game.LogNever)
	ct.add("east", ct.doEast, game.PosStanding, 0, game.LogNever)
	ct.add("south", ct.doSouth, game.PosStanding, 0, game.LogNever)
	ct.add("west", ct.doWest, game.PosStanding, 0, game.LogNever)
	ct.add("up", ct.doUp, game.PosStanding, 0, game.LogNever)
	ct.add("down", ct.doDown, game.PosStanding, 0, game.LogNever)

	// Common commands
	ct.add("look", ct.doLook, game.PosResting, 0, game.LogNormal)
	ct.add("exits", ct.doExits, game.PosResting, 0, game.LogNormal)
	ct.add("say", ct.doSay, game.PosResting, 0, game.LogNormal)
	ct.add("'", ct.doSay, game.PosResting, 0, game.LogNormal)
	ct.add("tell", ct.doTell, game.PosResting, 0, game.LogNormal)
	ct.add("who", ct.doWho, game.PosDead, 0, game.LogNormal)
	ct.add("save", ct.doSave, game.PosDead, 0, game.LogNormal)
	ct.add("quit", ct.doQuit, game.PosDead, 0, game.LogNormal)
	ct.add("help", ct.doHelp, game.PosDead, 0, game.LogNormal)

	return ct
}

func (ct *CommandTable) add(name string, fn CmdFunc, pos, level, logType int) {
	ct.entries = append(ct.entries, &CmdEntry{
		Name:     name,
		DoFun:    fn,
		Position: pos,
		Level:    level,
		Log:      logType,
	})
}

// Interpret parses and dispatches a command.
func (ct *CommandTable) Interpret(ch *game.CharData, argument string) {
	argument = strings.TrimLeft(argument, " \t")
	if argument == "" {
		return
	}

	// Grab command word
	var command string
	if !unicode.IsLetter(rune(argument[0])) && !unicode.IsDigit(rune(argument[0])) {
		command = string(argument[0])
		argument = strings.TrimLeft(argument[1:], " \t")
	} else {
		command, argument = game.OneArgument(argument)
	}

	// Look up in command table
	for _, cmd := range ct.entries {
		if len(cmd.Name) > 0 && cmd.Name[0] == command[0] && game.StrPrefix(command, cmd.Name) {
			if ch.Position < cmd.Position {
				ch.SendToChar("You can't do that right now.\n\r")
				return
			}
			if ch.Level < cmd.Level {
				ch.SendToChar("Huh?\n\r")
				return
			}
			if cmd.Log == game.LogAlways {
				log.Printf("Log %s: %s %s", ch.Name, command, argument)
			}
			cmd.DoFun(ch, argument)
			return
		}
	}

	ch.SendToChar("Huh?\n\r")
}

// --- Movement commands ---

func (ct *CommandTable) moveChar(ch *game.CharData, door int) {
	if ch.InRoom == nil {
		return
	}

	pexit := ch.InRoom.Exits[door]
	if pexit == nil || pexit.ToRoom == nil {
		ch.SendToChar("Alas, you cannot go that way.\n\r")
		return
	}

	toRoom := pexit.ToRoom

	if game.IsSet(pexit.ExitInfo, game.ExClosed) {
		keyword := pexit.Keyword
		if keyword == "" {
			keyword = "door"
		}
		game.Act("The $d is closed.", ch, nil, keyword, game.ToChar)
		return
	}

	if handler.RoomIsPrivate(toRoom) {
		ch.SendToChar("That room is private right now.\n\r")
		return
	}

	game.Act("$n leaves $T.", ch, nil, game.DirName[door], game.ToRoom)

	handler.CharFromRoom(ch)
	handler.CharToRoom(ch, toRoom, ct.world)

	game.Act("$n has arrived.", ch, nil, nil, game.ToRoom)
	ct.doLook(ch, "auto")
}

func (ct *CommandTable) doNorth(ch *game.CharData, argument string) { ct.moveChar(ch, game.DirNorth) }
func (ct *CommandTable) doEast(ch *game.CharData, argument string)  { ct.moveChar(ch, game.DirEast) }
func (ct *CommandTable) doSouth(ch *game.CharData, argument string) { ct.moveChar(ch, game.DirSouth) }
func (ct *CommandTable) doWest(ch *game.CharData, argument string)  { ct.moveChar(ch, game.DirWest) }
func (ct *CommandTable) doUp(ch *game.CharData, argument string)    { ct.moveChar(ch, game.DirUp) }
func (ct *CommandTable) doDown(ch *game.CharData, argument string)  { ct.moveChar(ch, game.DirDown) }

// --- Info commands ---

func (ct *CommandTable) doLook(ch *game.CharData, argument string) {
	if ch.InRoom == nil || ch.Desc == nil {
		return
	}

	if ch.Position < game.PosSleeping {
		ch.SendToChar("You can't see anything but stars!\n\r")
		return
	}
	if ch.Position == game.PosSleeping {
		ch.SendToChar("You can't see anything, you're sleeping!\n\r")
		return
	}

	if argument == "" || strings.EqualFold(argument, "auto") {
		// Room name
		ch.SendToChar(fmt.Sprintf("`C%s`0\n\r", ch.InRoom.Name))

		// Description (unless brief + auto)
		if argument == "" || !game.IsSet(ch.Comm, game.CommBrief) {
			if ch.InRoom.Description != "" {
				ch.SendToChar(ch.InRoom.Description)
			}
		}

		// Auto-exits
		if game.IsSet(ch.Act, game.PlrAutoExit) {
			ct.showExitsToChar(ch)
		}

		// People in room
		ct.showCharToChar(ch)
	}
}

func (ct *CommandTable) showExitsToChar(ch *game.CharData) {
	buf := "`C[Exits:"
	found := false

	for door := 0; door < game.MaxDir; door++ {
		pexit := ch.InRoom.Exits[door]
		if pexit != nil && pexit.ToRoom != nil && handler.CanSeeRoom(ch, pexit.ToRoom) {
			found = true
			if game.IsSet(pexit.ExitInfo, game.ExClosed) {
				buf += " (" + game.DirName[door] + ")"
			} else {
				buf += " " + game.DirName[door]
			}
		}
	}

	if !found {
		buf += " none"
	}
	buf += "]`0\n\r"
	ch.SendToChar(buf)
}

func (ct *CommandTable) showCharToChar(ch *game.CharData) {
	for _, rch := range ch.InRoom.People {
		if rch == ch {
			continue
		}
		if !handler.CanSee(ch, rch) {
			continue
		}

		if rch.LongDescr != "" && rch.Position == game.PosStanding && rch.IsNPC() {
			ch.SendToChar(rch.LongDescr)
		} else {
			posStr := ""
			switch rch.Position {
			case game.PosResting:
				posStr = " (Resting)"
			case game.PosSleeping:
				posStr = " (Sleeping)"
			case game.PosFighting:
				posStr = " (Fighting)"
			case game.PosSitting:
				posStr = " (Sitting)"
			}
			ch.SendToChar(fmt.Sprintf("%s%s is here.\n\r", rch.Name, posStr))
		}
	}
}

func (ct *CommandTable) doExits(ch *game.CharData, argument string) {
	if ch.InRoom == nil {
		return
	}

	found := false
	for door := 0; door < game.MaxDir; door++ {
		pexit := ch.InRoom.Exits[door]
		if pexit != nil && pexit.ToRoom != nil && handler.CanSeeRoom(ch, pexit.ToRoom) {
			found = true
			if game.IsSet(pexit.ExitInfo, game.ExClosed) {
				ch.SendToChar(fmt.Sprintf("%-5s - (closed)\n\r", game.Capitalize(game.DirName[door])))
			} else {
				name := pexit.ToRoom.Name
				if name == "" {
					name = "(no name)"
				}
				ch.SendToChar(fmt.Sprintf("%-5s - %s\n\r", game.Capitalize(game.DirName[door]), name))
			}
		}
	}

	if !found {
		ch.SendToChar("None.\n\r")
	}
}

// --- Communication commands ---

func (ct *CommandTable) doSay(ch *game.CharData, argument string) {
	if argument == "" {
		ch.SendToChar("Say what?\n\r")
		return
	}
	game.ActNew(game.CfgSay, ch, argument, nil, game.ToRoom, game.PosResting)
	game.ActNew(game.CfgSaySelf, ch, argument, nil, game.ToChar, game.PosResting)
}

func (ct *CommandTable) doTell(ch *game.CharData, argument string) {
	arg, rest := game.OneArgument(argument)
	if arg == "" || rest == "" {
		ch.SendToChar("Tell whom what?\n\r")
		return
	}

	victim := handler.GetPlayerWorld(ch, arg, ct.world)
	if victim == nil {
		ch.SendToChar("They aren't here.\n\r")
		return
	}

	if victim.Desc == nil && !victim.IsNPC() {
		game.Act("$N seems to have misplaced $S link...try again later.", ch, nil, victim, game.ToChar)
		return
	}

	game.ActNew("$n tells you '$t'", ch, rest, victim, game.ToVict, game.PosDead)
	ch.SendToChar(fmt.Sprintf("You tell %s '%s'\n\r", victim.Name, rest))
}

// --- Who / Help / Save / Quit ---

func (ct *CommandTable) doWho(ch *game.CharData, argument string) {
	ch.SendToChar("`W---[ Who is Online ]---`0\n\r")

	count := 0
	for _, d := range ct.world.Descriptors {
		if d.GetConnected() != game.ConPlaying {
			continue
		}
		wch := d.GetCharacter()
		if wch == nil {
			continue
		}
		if !handler.CanSee(ch, wch) {
			continue
		}
		count++
		title := ""
		if wch.PCData != nil && wch.PCData.Title != "" {
			title = wch.PCData.Title
		}
		ch.SendToChar(fmt.Sprintf("[%3d] %s%s\n\r", wch.Level, wch.Name, title))
	}

	suffix := "s"
	if count == 1 {
		suffix = ""
	}
	ch.SendToChar(fmt.Sprintf("\n\r%d player%s online.\n\r", count, suffix))
}

// SaveFn is set by main to wire in the persist package.
var SaveFn func(ch *game.CharData)

func (ct *CommandTable) doQuit(ch *game.CharData, argument string) {
	if ch.IsNPC() {
		return
	}
	if ch.Position == game.PosFighting {
		ch.SendToChar("No way! You are fighting.\n\r")
		return
	}

	ch.SendToChar(game.CfgQuit)
	game.Act("$n has left the game.", ch, nil, nil, game.ToRoom)
	log.Printf("%s has quit.", ch.Name)

	if SaveFn != nil {
		SaveFn(ch)
	}

	d := ch.Desc
	handler.ExtractChar(ch, true, ct.world)

	if d != nil {
		d.Close()
	}
}

func (ct *CommandTable) doSave(ch *game.CharData, argument string) {
	if ch.IsNPC() {
		return
	}
	if SaveFn != nil {
		SaveFn(ch)
	}
	ch.SendToChar("Saved.\n\r")
}

func (ct *CommandTable) doHelp(ch *game.CharData, argument string) {
	if argument == "" {
		argument = "summary"
	}

	h := ct.world.FindHelp(ch, argument)
	if h == nil {
		ch.SendToChar("No help on that word.\n\r")
		return
	}

	text := h.Text
	if len(text) > 0 && text[0] == '.' {
		text = text[1:]
	}
	ch.SendToChar(text)
}
