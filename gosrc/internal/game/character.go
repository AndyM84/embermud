package game

import "time"

// CharData represents a player or NPC character.
type CharData struct {
	Name        string
	ShortDescr  string
	LongDescr   string
	Description string
	Sex         int
	Level       int
	Trust       int
	Played      int // seconds
	Lines       int
	Logon       time.Time
	Timer       int
	Wait        int
	Act         int64 // PLR_ flags
	Comm        int64 // COMM_ flags
	InvisLevel  int
	Position    int

	Desc      Descriptor // nil if linkdead/NPC
	InRoom    *RoomIndex
	WasInRoom *RoomIndex
	PCData    *PCData // nil for NPCs
}

// Descriptor is forward-declared here to avoid import cycles.
// The actual Descriptor type lives in the network package.
// We use an interface to break the cycle.
type Descriptor = interface {
	WriteToBuffer(txt string)
	Close()
	GetConnected() int
	SetConnected(state int)
	GetAnsi() bool
	SetAnsi(v bool)
	GetCharacter() *CharData
	SetCharacter(ch *CharData)
	GetHost() string
	GetShowStrPoint() int
	SetShowStrPoint(v int)
	GetShowStrHead() string
	SetShowStrHead(v string)
}

// PCData holds player-specific data (nil for NPCs).
type PCData struct {
	Password   string
	Title      string
	Prompt     string
	Aliases    map[string]string // key=alias, value=substitution
	ConfirmDel bool
}

// NewPCData creates a PCData with defaults.
func NewPCData() *PCData {
	return &PCData{
		Prompt:  "> ",
		Aliases: make(map[string]string),
	}
}

// NewCharData creates a CharData with defaults.
func NewCharData() *CharData {
	return &CharData{
		Lines:    22,
		Position: PosStanding,
		Logon:    time.Now(),
	}
}

// IsNPC returns true if the character is an NPC.
func (ch *CharData) IsNPC() bool {
	return IsSet(ch.Act, PlrIsNPC)
}

// IsImmortal returns true if the character's trust >= LevelImmortal.
func (ch *CharData) IsImmortal() bool {
	return ch.GetTrust() >= LevelImmortal
}

// IsHero returns true if the character's trust >= LevelHero.
func (ch *CharData) IsHero() bool {
	return ch.GetTrust() >= LevelHero
}

// GetTrust returns the character's effective trust level.
func (ch *CharData) GetTrust() int {
	if ch.Trust != 0 {
		return ch.Trust
	}
	return ch.Level
}

// IsAwake returns true if the character is above sleeping position.
func (ch *CharData) IsAwake() bool {
	return ch.Position > PosSleeping
}

// SendToChar sends text to the character's descriptor.
func (ch *CharData) SendToChar(txt string) {
	if ch.Desc != nil {
		ch.Desc.WriteToBuffer(txt)
	}
}
