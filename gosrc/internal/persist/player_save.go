package persist

import (
	"bufio"
	"embermud/internal/game"
	"fmt"
	"log"
	"os"
	"strings"
	"time"
)

// SaveCharObj saves a player character to disk.
func SaveCharObj(ch *game.CharData, w *game.World) {
	if ch.IsNPC() {
		return
	}

	strsave := fmt.Sprintf("%s%s", w.PlayerDir, game.Capitalize(ch.Name))
	strtmp := w.PlayerTemp

	f, err := os.Create(strtmp)
	if err != nil {
		log.Printf("save_char_obj: %v", err)
		return
	}

	writeChar(ch, f, w)
	fmt.Fprintf(f, "#END\n")
	f.Close()

	os.Remove(strsave)
	os.Rename(strtmp, strsave)
}

func writeChar(ch *game.CharData, f *os.File, w *game.World) {
	fmt.Fprintf(f, "#PLAYER\n")

	fmt.Fprintf(f, "Name %s~\n", ch.Name)
	fmt.Fprintf(f, "ShD  %s~\n", ch.ShortDescr)
	fmt.Fprintf(f, "LnD  %s~\n", ch.LongDescr)
	fmt.Fprintf(f, "Desc %s~\n", ch.Description)
	fmt.Fprintf(f, "Sex  %d\n", ch.Sex)
	fmt.Fprintf(f, "Levl %d\n", ch.Level)
	fmt.Fprintf(f, "Tru  %d\n", ch.Trust)
	fmt.Fprintf(f, "Plyd %d\n", ch.Played+int(time.Since(ch.Logon).Seconds()))
	fmt.Fprintf(f, "Scro %d\n", ch.Lines)

	roomVnum := game.RoomVnumTemple
	if ch.InRoom != nil {
		roomVnum = ch.InRoom.Vnum
	}
	fmt.Fprintf(f, "Room %d\n", roomVnum)
	fmt.Fprintf(f, "Act  %d\n", ch.Act)
	fmt.Fprintf(f, "Comm %d\n", ch.Comm)

	pos := ch.Position
	if pos == game.PosFighting {
		pos = game.PosStanding
	}
	fmt.Fprintf(f, "Pos  %d\n", pos)
	fmt.Fprintf(f, "InvL %d\n", ch.InvisLevel)

	if ch.PCData != nil {
		fmt.Fprintf(f, "Pass %s~\n", ch.PCData.Password)
		fmt.Fprintf(f, "Titl %s~\n", ch.PCData.Title)
		fmt.Fprintf(f, "Prom %s~\n", ch.PCData.Prompt)

		for alias, sub := range ch.PCData.Aliases {
			fmt.Fprintf(f, "Alias %s~ %s~\n", alias, sub)
		}
	}

	fmt.Fprintf(f, "End\n\n")
}

// LoadCharObj loads a player character from disk.
// Returns true if an existing player file was found.
func LoadCharObj(d game.Descriptor, name string, w *game.World) bool {
	ch := game.NewCharData()
	ch.Name = name
	ch.PCData = game.NewPCData()

	d.SetCharacter(ch)
	ch.Desc = d

	strsave := fmt.Sprintf("%s%s", w.PlayerDir, game.Capitalize(name))

	f, err := os.Open(strsave)
	if err != nil {
		return false
	}
	defer f.Close()

	r := bufio.NewReader(f)
	found := false

	for {
		c := FreadLetter(r)
		if c == 0 {
			break
		}
		if c == '#' {
			word := FreadWord(r)
			if strings.EqualFold(word, "PLAYER") {
				readChar(ch, r, w)
				found = true
			} else if strings.EqualFold(word, "END") {
				break
			}
		}
	}

	return found
}

func readChar(ch *game.CharData, r *bufio.Reader, w *game.World) {
	for {
		word := FreadWord(r)
		if word == "" {
			break
		}

		upper := strings.ToUpper(word)

		if upper == "END" {
			return
		}

		switch upper {
		case "ACT":
			ch.Act = int64(FreadNumber(r))
		case "ALIAS":
			alias := FreadString(r)
			sub := FreadString(r)
			if ch.PCData != nil && len(ch.PCData.Aliases) < game.MaxAlias {
				ch.PCData.Aliases[alias] = sub
			}
		case "COMM":
			ch.Comm = int64(FreadNumber(r))
		case "DESC":
			ch.Description = FreadString(r)
		case "INVL":
			ch.InvisLevel = FreadNumber(r)
		case "LEVL":
			ch.Level = FreadNumber(r)
		case "LND":
			ch.LongDescr = FreadString(r)
		case "NAME":
			ch.Name = FreadString(r)
		case "PASS":
			if ch.PCData != nil {
				ch.PCData.Password = FreadString(r)
			}
		case "PLYD":
			ch.Played = FreadNumber(r)
		case "POS":
			ch.Position = FreadNumber(r)
		case "PROM":
			if ch.PCData != nil {
				ch.PCData.Prompt = FreadString(r)
			}
		case "ROOM":
			vnum := FreadNumber(r)
			room := w.GetRoomIndex(vnum)
			if room == nil {
				room = w.GetRoomIndex(game.RoomVnumTemple)
			}
			ch.InRoom = room
		case "SEX":
			ch.Sex = FreadNumber(r)
		case "SHD":
			ch.ShortDescr = FreadString(r)
		case "SCRO":
			ch.Lines = FreadNumber(r)
		case "TRU":
			ch.Trust = FreadNumber(r)
		case "TITL":
			if ch.PCData != nil {
				ch.PCData.Title = FreadString(r)
			}
		default:
			log.Printf("fread_char: no match for '%s'", word)
			FreadToEol(r)
		}
	}
}
