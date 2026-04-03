package game

import (
	"fmt"
	"log"
	"strings"
	"time"
	"unicode"
)

// World holds all global game state.
type World struct {
	Rooms        map[int]*RoomIndex
	Characters   []*CharData
	Players      []*CharData
	Descriptors  []Descriptor
	Helps        []*HelpData
	Bans         []*BanData
	HelpGreeting string
	BootTime     time.Time
	MercDown     bool
	Wizlock      bool
	Newlock      bool
	CurrentTime  time.Time

	AreaDir    string
	PlayerDir  string
	PlayerTemp string
	LogDir     string
}

// NewWorld creates a new World with defaults.
func NewWorld() *World {
	return &World{
		Rooms:      make(map[int]*RoomIndex),
		BootTime:   time.Now(),
		CurrentTime: time.Now(),
		AreaDir:    CfgAreaDir,
		PlayerDir:  CfgPlayerDir,
		PlayerTemp: CfgPlayerTemp,
		LogDir:     CfgLogDir,
	}
}

// GetRoomIndex looks up a room by vnum.
func (w *World) GetRoomIndex(vnum int) *RoomIndex {
	return w.Rooms[vnum]
}

// AddRoom adds a room to the world.
func (w *World) AddRoom(room *RoomIndex) {
	w.Rooms[room.Vnum] = room
}

// AddCharacter adds a character to the global character list.
func (w *World) AddCharacter(ch *CharData) {
	w.Characters = append(w.Characters, ch)
}

// RemoveCharacter removes a character from the global character list.
func (w *World) RemoveCharacter(ch *CharData) {
	for i, c := range w.Characters {
		if c == ch {
			w.Characters = append(w.Characters[:i], w.Characters[i+1:]...)
			return
		}
	}
}

// AddPlayer adds a player to the player list.
func (w *World) AddPlayer(ch *CharData) {
	w.Players = append(w.Players, ch)
}

// RemovePlayer removes a player from the player list.
func (w *World) RemovePlayer(ch *CharData) {
	for i, p := range w.Players {
		if p == ch {
			w.Players = append(w.Players[:i], w.Players[i+1:]...)
			return
		}
	}
}

// AddDescriptor adds a descriptor to the list.
func (w *World) AddDescriptor(d Descriptor) {
	w.Descriptors = append(w.Descriptors, d)
}

// RemoveDescriptor removes a descriptor from the list.
func (w *World) RemoveDescriptor(d Descriptor) {
	for i, desc := range w.Descriptors {
		if desc == d {
			w.Descriptors = append(w.Descriptors[:i], w.Descriptors[i+1:]...)
			return
		}
	}
}

// AddHelp adds a help entry.
func (w *World) AddHelp(h *HelpData) {
	w.Helps = append(w.Helps, h)
	kw := strings.ToLower(h.Keyword)
	if kw == "greeting" || kw == "ansigreet" {
		w.HelpGreeting = h.Text
	}
}

// FindHelp finds a help entry by keyword.
func (w *World) FindHelp(ch *CharData, keyword string) *HelpData {
	for _, h := range w.Helps {
		if h.Level > ch.GetTrust() {
			continue
		}
		if IsName(keyword, h.Keyword) {
			return h
		}
	}
	return nil
}

// CreateMinimalWorld creates a hardcoded 8-room world.
func (w *World) CreateMinimalWorld() {
	makeRoom := func(vnum int, name, desc string, flags int64, sector int) *RoomIndex {
		r := &RoomIndex{
			Vnum:        vnum,
			Name:        name,
			Description: desc,
			RoomFlags:   flags,
			SectorType:  sector,
		}
		w.AddRoom(r)
		return r
	}

	makeExit := func(from *RoomIndex, dir int, to *RoomIndex) {
		from.Exits[dir] = &ExitData{ToRoom: to}
	}

	limbo := makeRoom(RoomVnumLimbo, "Limbo",
		"You are floating in an endless void of grey nothingness.\n\r"+
			"There is no ground beneath your feet, no sky above your head.\n\r"+
			"The silence here is absolute and oppressive, broken only by\n\r"+
			"the faint echo of your own thoughts.\n\r",
		RoomSafe, SectInside)

	square := makeRoom(RoomVnumTemple, "Town Square",
		"You stand at the heart of a small medieval town. Cobblestones\n\r"+
			"stretch in every direction, worn smooth by centuries of foot\n\r"+
			"traffic. A weathered stone fountain sits in the center of the\n\r"+
			"square, its basin filled with clear water. Roads lead north,\n\r"+
			"south, east, and west, while a spiral staircase leads up to a\n\r"+
			"watchtower and a trapdoor opens into a cellar below.\n\r",
		RoomSafe|RoomNoMob, SectCity)

	northRoad := makeRoom(3002, "North Road",
		"A hard-packed dirt road stretches northward into rolling green\n\r"+
			"hills. Wooden fences line both sides of the path, enclosing\n\r"+
			"small pastures where sheep graze lazily. The town square lies\n\r"+
			"to the south, its fountain just visible over the rooftops.\n\r",
		0, SectField)

	southGate := makeRoom(3003, "South Gate",
		"A massive wooden gate reinforced with iron bands marks the\n\r"+
			"southern entrance to the town. Two stone pillars flank the\n\r"+
			"gateway, each topped with a flickering torch. Beyond the\n\r"+
			"gate, a dusty road winds away into dark, dense forest.\n\r",
		0, SectCity)

	eastMarket := makeRoom(3004, "East Market",
		"Rows of wooden stalls and canvas-covered booths fill this\n\r"+
			"bustling marketplace. The air carries the mingled scents of\n\r"+
			"fresh bread, spiced meats, and exotic herbs. Colorful banners\n\r"+
			"hang between the stalls, snapping gently in the breeze.\n\r",
		0, SectCity)

	westGarden := makeRoom(3005, "West Garden",
		"A tranquil garden occupies this corner of town, enclosed by\n\r"+
			"a low stone wall covered in climbing ivy. Ancient oak trees\n\r"+
			"provide dappled shade over beds of wildflowers and medicinal\n\r"+
			"herbs. A moss-covered stone bench invites quiet reflection.\n\r",
		RoomSafe, SectField)

	watchTower := makeRoom(3006, "Watch Tower",
		"You stand atop a tall stone watchtower that rises above the\n\r"+
			"town rooftops. From this vantage point, the entire settlement\n\r"+
			"spreads below you -- the square, the market, the garden, and\n\r"+
			"the roads leading away into the countryside. A cold wind whips\n\r"+
			"at your clothing. A spiral staircase leads back down.\n\r",
		RoomIndoors, SectInside)

	cellar := makeRoom(3007, "Cellar",
		"A damp, cool cellar stretches beneath the town square. Thick\n\r"+
			"stone walls are lined with wooden shelves holding dusty bottles\n\r"+
			"and forgotten provisions. Cobwebs drape from the low ceiling,\n\r"+
			"and the only light filters down from the trapdoor above.\n\r",
		RoomDark|RoomIndoors, SectInside)

	_ = limbo

	// Square exits
	makeExit(square, DirNorth, northRoad)
	makeExit(square, DirSouth, southGate)
	makeExit(square, DirEast, eastMarket)
	makeExit(square, DirWest, westGarden)
	makeExit(square, DirUp, watchTower)
	makeExit(square, DirDown, cellar)

	// Return exits
	makeExit(northRoad, DirSouth, square)
	makeExit(southGate, DirNorth, square)
	makeExit(eastMarket, DirWest, square)
	makeExit(westGarden, DirEast, square)
	makeExit(watchTower, DirDown, square)
	makeExit(cellar, DirUp, square)

	log.Println("Minimal world created: 8 rooms, 12 exits.")
}

// CreateBasicHelps creates built-in help entries.
func (w *World) CreateBasicHelps() {
	w.AddHelp(&HelpData{Level: 0, Keyword: "MOTD",
		Text: "Welcome back!  This MUD is currently in development.\n\r" +
			"Please report any bugs you find.\n\r"})

	w.AddHelp(&HelpData{Level: 0, Keyword: "RULES",
		Text: "1. Be respectful to other players.\n\r" +
			"2. No harassment or abuse.\n\r" +
			"3. Have fun!\n\r"})

	w.AddHelp(&HelpData{Level: 0, Keyword: "COMMANDS SUMMARY",
		Text: "`WAvailable Commands:`0\n\r" +
			"  look        - Look at your surroundings\n\r" +
			"  exits       - Show available exits\n\r" +
			"  north south east west up down - Move in a direction\n\r" +
			"  say <msg>   - Say something to the room\n\r" +
			"  tell <who> <msg> - Send a private message\n\r" +
			"  who         - List online players\n\r" +
			"  save        - Save your character\n\r" +
			"  quit        - Save and leave the game\n\r" +
			"  help <topic>- Get help on a topic\n\r"})

	if w.HelpGreeting == "" {
		w.HelpGreeting = "\n\r`WWelcome to EmberMUD!`w\n\r\n\r" +
			"Based on ROM 2.4, Merc 2.1, and DikuMUD.\n\r" +
			"Type your character name to begin.\n\r\n\r"
	}
}

// Boot initializes the world.
func (w *World) Boot() {
	w.Bans = LoadBans(CfgBanFile)

	// Try loading area files, fall back to minimal world.
	if AreaLoader != nil {
		AreaLoader(w)
	}

	if w.GetRoomIndex(RoomVnumLimbo) == nil && w.GetRoomIndex(RoomVnumTemple) == nil {
		log.Println("No rooms loaded from area files, using hardcoded world.")
		w.CreateMinimalWorld()
	}

	if len(w.Helps) == 0 {
		log.Println("No help entries loaded from area files, using built-in helps.")
		w.CreateBasicHelps()
	}

	if w.HelpGreeting == "" {
		w.CreateBasicHelps()
	}

	nRooms := len(w.Rooms)
	nExits := 0
	for _, r := range w.Rooms {
		for _, e := range r.Exits {
			if e != nil {
				nExits++
			}
		}
	}
	log.Printf("Boot complete: %d rooms, %d exits, %d help entries.", nRooms, nExits, len(w.Helps))
}

// AreaLoader is set by the persist package to load area files.
// This allows World to be compiled without importing persist.
var AreaLoader func(w *World)

// Utility functions that mirror the C helpers.

// Capitalize returns a string with the first letter uppercased.
func Capitalize(s string) string {
	if s == "" {
		return s
	}
	runes := []rune(s)
	runes[0] = unicode.ToUpper(runes[0])
	return string(runes)
}

// StrPrefix returns true if astr is a prefix of bstr (case-insensitive).
func StrPrefix(astr, bstr string) bool {
	a := strings.ToLower(astr)
	b := strings.ToLower(bstr)
	return strings.HasPrefix(b, a)
}

// StrCmp is case-insensitive string comparison. Returns true if NOT equal (like C).
func StrCmp(a, b string) bool {
	return !strings.EqualFold(a, b)
}

// StrSuffix returns true if astr is a suffix of bstr (case-insensitive).
func StrSuffix(astr, bstr string) bool {
	a := strings.ToLower(astr)
	b := strings.ToLower(bstr)
	return strings.HasSuffix(b, a)
}

// IsName checks if str matches any word in namelist (prefix match).
func IsName(str, namelist string) bool {
	if str == "" || namelist == "" {
		return false
	}

	// Extract each word from str and check against namelist
	remaining := str
	for {
		var part string
		part, remaining = OneArgument(remaining)
		if part == "" {
			return true
		}

		// Check if part matches any word in namelist
		found := false
		nl := namelist
		for {
			var name string
			name, nl = OneArgument(nl)
			if name == "" {
				break
			}
			if StrPrefix(part, name) {
				found = true
				break
			}
		}
		if !found {
			return false
		}
	}
}

// IsExactName checks if str exactly matches any word in namelist.
func IsExactName(str, namelist string) bool {
	if str == "" || namelist == "" {
		return false
	}

	remaining := str
	for {
		var part string
		part, remaining = OneArgument(remaining)
		if part == "" {
			return true
		}

		found := false
		nl := namelist
		for {
			var name string
			name, nl = OneArgument(nl)
			if name == "" {
				break
			}
			if strings.EqualFold(part, name) {
				found = true
				break
			}
		}
		if !found {
			return false
		}
	}
}

// OneArgument extracts one word from argument, lowercased.
// Returns (word, rest).
func OneArgument(argument string) (string, string) {
	argument = strings.TrimLeft(argument, " \t")

	if argument == "" {
		return "", ""
	}

	cEnd := ' '
	if argument[0] == '\'' {
		cEnd = '\''
		argument = argument[1:]
	}

	var word []byte
	i := 0
	for i < len(argument) {
		if rune(argument[i]) == rune(cEnd) {
			i++
			break
		}
		word = append(word, byte(unicode.ToLower(rune(argument[i]))))
		i++
	}

	rest := strings.TrimLeft(argument[i:], " \t")
	return string(word), rest
}

// NumberArgument parses "N.keyword" syntax.
// Returns (number, keyword).
func NumberArgument(argument string) (int, string) {
	dotIdx := strings.IndexByte(argument, '.')
	if dotIdx < 0 {
		return 1, argument
	}

	numStr := argument[:dotIdx]
	keyword := argument[dotIdx+1:]
	num := 1
	fmt.Sscanf(numStr, "%d", &num)
	return num, keyword
}

// CheckParseName validates a character name.
func CheckParseName(name string) bool {
	reserved := "all auto immortal self someone something the you"
	if IsExactName(strings.ToLower(name), reserved) {
		return false
	}

	if len(name) < 2 || len(name) > 12 {
		return false
	}

	allIL := true
	for _, c := range name {
		if !unicode.IsLetter(c) {
			return false
		}
		lc := unicode.ToLower(c)
		if lc != 'i' && lc != 'l' {
			allIL = false
		}
	}

	if allIL {
		return false
	}

	return true
}

// SendToRoom sends text to all characters in a room with descriptors.
func SendToRoom(txt string, room *RoomIndex) {
	if room == nil {
		return
	}
	for _, ch := range room.People {
		ch.SendToChar(txt)
	}
}
