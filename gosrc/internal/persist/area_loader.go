package persist

import (
	"bufio"
	"embermud/internal/game"
	"fmt"
	"log"
	"os"
	"strings"
	"unicode"
)

// LoadAreaFiles loads all area files listed in area.lst.
func LoadAreaFiles(w *game.World) {
	listPath := fmt.Sprintf("%s%s", w.AreaDir, "area.lst")
	f, err := os.Open(listPath)
	if err != nil {
		log.Printf("load_area_files: %s not found, using hardcoded world.", listPath)
		return
	}
	defer f.Close()

	r := bufio.NewReader(f)
	for {
		word := FreadWord(r)
		if word == "" || word[0] == '$' {
			break
		}
		filename := fmt.Sprintf("%s%s", w.AreaDir, word)
		loadAreaFile(filename, w)
	}
}

func loadAreaFile(filename string, w *game.World) {
	f, err := os.Open(filename)
	if err != nil {
		log.Printf("load_area_file: could not open %s, skipping.", filename)
		return
	}
	defer f.Close()

	r := bufio.NewReader(f)

	for {
		letter := FreadLetter(r)
		if letter == 0 {
			break
		}
		if letter != '#' {
			log.Printf("load_area_file: # not found in %s.", filename)
			break
		}

		word := FreadWord(r)
		if word == "" || word[0] == '$' {
			break
		}

		upper := strings.ToUpper(word)
		switch upper {
		case "AREADATA":
			skipSectionAreadata(r)
		case "HELPS":
			loadHelps(r, w)
		case "MOBILES":
			skipSectionVnumList(r)
		case "OBJECTS":
			skipSectionVnumList(r)
		case "ROOMS":
			loadRooms(r, w)
		case "RESETS":
			skipSectionWordUntilS(r)
		case "SHOPS":
			skipSectionShops(r)
		case "PROGS":
			skipSectionWordUntilS(r)
		default:
			log.Printf("load_area_file: unknown section '%s' in %s, skipping.", word, filename)
			skipSectionUnknown(r)
		}
	}
}

func loadRooms(r *bufio.Reader, w *game.World) {
	for {
		letter := FreadLetter(r)
		if letter != '#' {
			log.Println("load_rooms: # not found.")
			return
		}

		vnum := FreadNumber(r)
		if vnum == 0 {
			break
		}

		if w.GetRoomIndex(vnum) != nil {
			log.Printf("load_rooms: duplicate vnum %d.", vnum)
			continue
		}

		room := &game.RoomIndex{
			Vnum:        vnum,
			Name:        FreadString(r),
			Description: FreadString(r),
		}

		_ = FreadNumber(r) // area_num
		room.RoomFlags = FreadFlag(r)
		room.SectorType = FreadNumber(r)

		for {
			letter = FreadLetter(r)

			if letter == 'S' {
				break
			}

			if letter == 'D' {
				door := FreadNumber(r)
				if door < 0 || door > 5 {
					log.Printf("load_rooms: bad door %d in vnum %d.", door, vnum)
					continue
				}

				pexit := &game.ExitData{
					Description: FreadString(r),
					Keyword:     FreadString(r),
				}

				locks := FreadNumber(r)
				pexit.Key = FreadNumber(r)
				pexit.ToVnum = FreadNumber(r)

				switch locks {
				case 0:
					pexit.ExitInfo = 0
				case 1:
					pexit.ExitInfo = game.ExIsDoor
				case 2:
					pexit.ExitInfo = game.ExIsDoor | game.ExClosed | game.ExLocked | game.ExPickProof
				default:
					pexit.ExitInfo = game.ExIsDoor | game.ExClosed | game.ExLocked
				}

				room.Exits[door] = pexit
			} else if letter == 'E' {
				// Extra description — discard
				FreadString(r)
				FreadString(r)
			} else {
				log.Printf("load_rooms: unknown sub-record '%c' in vnum %d.", letter, vnum)
				return
			}
		}

		w.AddRoom(room)
	}
}

func loadHelps(r *bufio.Reader, w *game.World) {
	for {
		level := FreadNumber(r)
		keyword := FreadString(r)

		if keyword == "" || keyword[0] == '$' {
			break
		}

		text := FreadString(r)

		w.AddHelp(&game.HelpData{
			Level:   level,
			Keyword: keyword,
			Text:    text,
		})
	}
}

// FixExits resolves exit vnums to room pointers.
func FixExits(w *game.World) {
	nUnresolved := 0
	for _, room := range w.Rooms {
		for door := 0; door < game.MaxDir; door++ {
			pexit := room.Exits[door]
			if pexit == nil {
				continue
			}
			if pexit.ToVnum <= 0 {
				pexit.ToRoom = nil
			} else {
				pexit.ToRoom = w.GetRoomIndex(pexit.ToVnum)
				if pexit.ToRoom == nil {
					log.Printf("fix_exits: room %d exit %d -> vnum %d not found.",
						room.Vnum, door, pexit.ToVnum)
					nUnresolved++
				}
			}
		}
	}
	if nUnresolved > 0 {
		log.Printf("fix_exits: %d unresolved exit(s).", nUnresolved)
	}
}

// Section skippers

func skipSectionAreadata(r *bufio.Reader) {
	for {
		word := FreadWord(r)
		if strings.EqualFold(word, "End") {
			break
		}
		FreadToEol(r)
	}
}

func skipSectionVnumList(r *bufio.Reader) {
	atHash := false
	for {
		b, err := r.ReadByte()
		if err != nil {
			return
		}
		if b == '#' {
			atHash = true
			continue
		}
		if atHash && b == '0' {
			next, err := r.ReadByte()
			if err != nil || unicode.IsSpace(rune(next)) {
				if err == nil {
					r.UnreadByte()
				}
				return
			}
		}
		atHash = false
	}
}

func skipSectionWordUntilS(r *bufio.Reader) {
	for {
		word := FreadWord(r)
		if word == "S" {
			break
		}
		FreadToEol(r)
	}
}

func skipSectionShops(r *bufio.Reader) {
	for {
		keeper := FreadNumber(r)
		if keeper == 0 {
			break
		}
		FreadToEol(r)
	}
}

func skipSectionUnknown(r *bufio.Reader) {
	atLineStart := true
	for {
		b, err := r.ReadByte()
		if err != nil {
			return
		}
		if atLineStart && b == '#' {
			next, err := r.ReadByte()
			if err != nil {
				return
			}
			if unicode.IsUpper(rune(next)) || next == '$' {
				r.UnreadByte()
				r.UnreadByte()
				return
			}
		}
		atLineStart = (b == '\n' || b == '\r')
	}
}
