package game

import "time"

// Version string.
const EmberMUDVersion = "`WEmberMUD Core 1.0 (Go)`w\n\r"

// String and buffer limits.
const (
	MaxKeyHash       = 1024
	MaxStringLength  = 4608
	MaxInputLength   = 256
	MaxOutputBuffer  = 32768
	MaxAlias         = 20
)

// Game parameters.
const (
	MaxLevel      = 60
	LevelHero     = MaxLevel - 9
	LevelImmortal = MaxLevel - 8

	PulsePerSecond = 4
	PulseTick      = 60 * PulsePerSecond
	PulseDuration  = time.Second / PulsePerSecond
)

// Immortal level shortcuts.
const (
	Implementor = MaxLevel
	Creator     = MaxLevel - 1
	Supreme     = MaxLevel - 2
	Deity       = MaxLevel - 3
	God         = MaxLevel - 4
	Immortal    = MaxLevel - 5
	Demi        = MaxLevel - 6
	Angel       = MaxLevel - 7
	Avatar      = MaxLevel - 8
	Hero        = LevelHero
)

// Directions.
const (
	DirNorth = 0
	DirEast  = 1
	DirSouth = 2
	DirWest  = 3
	DirUp    = 4
	DirDown  = 5
	MaxDir   = 6
)

// Direction names and reverse lookup.
var DirName = [MaxDir]string{"north", "east", "south", "west", "up", "down"}
var RevDir = [MaxDir]int{DirSouth, DirWest, DirNorth, DirEast, DirDown, DirUp}

// Sex.
const (
	SexNeutral = 0
	SexMale    = 1
	SexFemale  = 2
)

// Positions.
const (
	PosDead     = 0
	PosMortal   = 1
	PosIncap    = 2
	PosStunned  = 3
	PosSleeping = 4
	PosResting  = 5
	PosSitting  = 6
	PosFighting = 7
	PosStanding = 8
)

// PLR (act) flags.
const (
	PlrIsNPC     int64 = 1 << 0
	PlrAutoExit  int64 = 1 << 1
	PlrAutoMap   int64 = 1 << 2
	PlrAutoLoot  int64 = 1 << 3
	PlrAutoSac   int64 = 1 << 4
	PlrAutoGold  int64 = 1 << 5
	PlrAutoSplit int64 = 1 << 6
	PlrHolyLight int64 = 1 << 13
	PlrWizInvis  int64 = 1 << 14
	PlrCanLoot   int64 = 1 << 15
	PlrNoSummon  int64 = 1 << 16
	PlrNoFollow  int64 = 1 << 17
	PlrColour    int64 = 1 << 18
	PlrDeny      int64 = 1 << 19
	PlrFreeze    int64 = 1 << 20
	PlrLog       int64 = 1 << 22
)

// COMM flags.
const (
	CommQuiet      int64 = 1 << 0
	CommDeaf       int64 = 1 << 1
	CommNoShout    int64 = 1 << 2
	CommNoTell     int64 = 1 << 3
	CommNoChannels int64 = 1 << 4
	CommCompact    int64 = 1 << 5
	CommBrief      int64 = 1 << 6
	CommPrompt     int64 = 1 << 7
	CommCombine    int64 = 1 << 8
	CommNoEmote    int64 = 1 << 9
	CommAFK        int64 = 1 << 10
)

// Room flags.
const (
	RoomDark       int64 = 1 << 0
	RoomNoMob      int64 = 1 << 2
	RoomIndoors    int64 = 1 << 3
	RoomPrivate    int64 = 1 << 9
	RoomSafe       int64 = 1 << 10
	RoomSolitary   int64 = 1 << 11
	RoomImpOnly    int64 = 1 << 14
	RoomGodsOnly   int64 = 1 << 15
	RoomHeroesOnly int64 = 1 << 16
	RoomNewbiesOnly int64 = 1 << 17
)

// Exit flags.
const (
	ExIsDoor    int64 = 1 << 0
	ExClosed    int64 = 1 << 1
	ExLocked    int64 = 1 << 2
	ExPickProof int64 = 1 << 5
	ExNoPass    int64 = 1 << 6
)

// Sector types.
const (
	SectInside      = 0
	SectCity        = 1
	SectField       = 2
	SectForest      = 3
	SectHills       = 4
	SectMountain    = 5
	SectWaterSwim   = 6
	SectWaterNoSwim = 7
	SectAir         = 9
	SectDesert      = 10
)

// Log types.
const (
	LogNormal = 0
	LogAlways = 1
	LogNever  = 2
)

// act() target types.
const (
	ToRoom    = 0
	ToNotVict = 1
	ToVict    = 2
	ToChar    = 3
)

// Connection states.
const (
	ConPlaying           = 0
	ConGetName           = 1
	ConGetOldPassword    = 2
	ConConfirmNewName    = 3
	ConGetNewPassword    = 4
	ConConfirmNewPassword = 5
	ConGetNewRole        = 6
	ConReadMotd          = 7
	ConBreakConnect      = 8
	ConGetAnsi           = 9
)

// Ban flags.
const (
	BanSuffix    = 1
	BanPrefix    = 2
	BanNewbies   = 4
	BanAll       = 8
	BanPermit    = 16
	BanPermanent = 32
)

// Special room vnums.
const (
	RoomVnumLimbo  = 2
	RoomVnumTemple = 3001
)

// Configuration messages.
const (
	CfgQuit       = "Alas, all good things must come to an end.\n\r"
	CfgConnectMsg = "Welcome to a MUD based on EmberMUD.\n\r"
	CfgAskAnsi    = "Use ANSI Color? [Y/n]: "
	CfgSay        = "`G$n says `g'`G$t`g'`0"
	CfgSaySelf    = "`GYou say `g'`G$t`g'`0"
)

// Default paths (relative to executable).
const (
	CfgAreaDir    = "../area/"
	CfgPlayerDir  = "../player/"
	CfgPlayerTemp = "../player/temp.tmp"
	CfgLogDir     = "../log/"
	CfgHelpFile   = "help.txt"
	CfgBugFile    = "../log/bugs.txt"
	CfgBanFile    = "../area/ban.txt"
)

// Bit manipulation helpers.
func IsSet(flag, bit int64) bool { return flag&bit != 0 }
func SetBit(flag *int64, bit int64) { *flag |= bit }
func RemoveBit(flag *int64, bit int64) { *flag &= ^bit }
