"""Configuration constants and defaults for EmberMUD."""

# Version string (backtick color codes from the C version)
EMBER_MUD_VERSION = "`WEmberMUD Core 1.0 (Python)`w\n\r"

# String and memory parameters
MAX_KEY_HASH = 1024
MAX_STRING_LENGTH = 4608
MAX_INPUT_LENGTH = 256
MAX_OUTPUT_BUFFER = 32768
MAX_ALIAS = 20

# Game parameters
MAX_LEVEL = 60
LEVEL_HERO = MAX_LEVEL - 9
LEVEL_IMMORTAL = MAX_LEVEL - 8

IMPLEMENTOR = MAX_LEVEL
CREATOR = MAX_LEVEL - 1
SUPREME = MAX_LEVEL - 2
DEITY = MAX_LEVEL - 3
GOD = MAX_LEVEL - 4
IMMORTAL = MAX_LEVEL - 5
DEMI = MAX_LEVEL - 6
ANGEL = MAX_LEVEL - 7
AVATAR = MAX_LEVEL - 8
HERO = LEVEL_HERO

# Timing
PULSE_PER_SECOND = 4
PULSE_TICK = 60 * PULSE_PER_SECOND

# Directions
DIR_NORTH = 0
DIR_EAST = 1
DIR_SOUTH = 2
DIR_WEST = 3
DIR_UP = 4
DIR_DOWN = 5
MAX_DIR = 6

dir_name = ["north", "east", "south", "west", "up", "down"]
rev_dir = [DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP]

# Sex
SEX_NEUTRAL = 0
SEX_MALE = 1
SEX_FEMALE = 2

# Positions
POS_DEAD = 0
POS_MORTAL = 1
POS_INCAP = 2
POS_STUNNED = 3
POS_SLEEPING = 4
POS_RESTING = 5
POS_SITTING = 6
POS_FIGHTING = 7
POS_STANDING = 8

# ACT bits for players (PLR flags)
PLR_IS_NPC = 1 << 0
PLR_AUTOEXIT = 1 << 1
PLR_AUTOMAP = 1 << 2
PLR_AUTOLOOT = 1 << 3
PLR_AUTOSAC = 1 << 4
PLR_AUTOGOLD = 1 << 5
PLR_AUTOSPLIT = 1 << 6
PLR_HOLYLIGHT = 1 << 13
PLR_WIZINVIS = 1 << 14
PLR_CANLOOT = 1 << 15
PLR_NOSUMMON = 1 << 16
PLR_NOFOLLOW = 1 << 17
PLR_COLOUR = 1 << 18
PLR_DENY = 1 << 19
PLR_FREEZE = 1 << 20
PLR_LOG = 1 << 22

# COMM bits
COMM_QUIET = 1 << 0
COMM_DEAF = 1 << 1
COMM_NOSHOUT = 1 << 2
COMM_NOTELL = 1 << 3
COMM_NOCHANNELS = 1 << 4
COMM_COMPACT = 1 << 5
COMM_BRIEF = 1 << 6
COMM_PROMPT = 1 << 7
COMM_COMBINE = 1 << 8
COMM_NOEMOTE = 1 << 9
COMM_AFK = 1 << 10

# Room flags
ROOM_DARK = 1 << 0
ROOM_NO_MOB = 1 << 2
ROOM_INDOORS = 1 << 3
ROOM_PRIVATE = 1 << 9
ROOM_SAFE = 1 << 10
ROOM_SOLITARY = 1 << 11
ROOM_IMP_ONLY = 1 << 14
ROOM_GODS_ONLY = 1 << 15
ROOM_HEROES_ONLY = 1 << 16
ROOM_NEWBIES_ONLY = 1 << 17

# Exit flags
EX_ISDOOR = 1 << 0
EX_CLOSED = 1 << 1
EX_LOCKED = 1 << 2
EX_PICKPROOF = 1 << 5
EX_NOPASS = 1 << 6

# Sector types
SECT_INSIDE = 0
SECT_CITY = 1
SECT_FIELD = 2
SECT_FOREST = 3
SECT_HILLS = 4
SECT_MOUNTAIN = 5
SECT_WATER_SWIM = 6
SECT_WATER_NOSWIM = 7
SECT_AIR = 9
SECT_DESERT = 10

# Log types
LOG_NORMAL = 0
LOG_ALWAYS = 1
LOG_NEVER = 2

# Target types for act()
TO_ROOM = 0
TO_NOTVICT = 1
TO_VICT = 2
TO_CHAR = 3

# Ban flags
BAN_SUFFIX = 1
BAN_PREFIX = 2
BAN_NEWBIES = 4
BAN_ALL = 8
BAN_PERMIT = 16
BAN_PERMANENT = 32

# Connection states
CON_PLAYING = 0
CON_GET_NAME = 1
CON_GET_OLD_PASSWORD = 2
CON_CONFIRM_NEW_NAME = 3
CON_GET_NEW_PASSWORD = 4
CON_CONFIRM_NEW_PASSWORD = 5
CON_GET_NEW_ROLE = 6
CON_READ_MOTD = 7
CON_BREAK_CONNECT = 8
CON_GET_ANSI = 9

# Special room vnums
ROOM_VNUM_LIMBO = 2
ROOM_VNUM_TEMPLE = 3001

# Connection/display messages
CFG_QUIT = "Alas, all good things must come to an end.\n\r"
CFG_CONNECT_MSG = "Welcome to a MUD based on EmberMUD.\n\r"
CFG_ASK_ANSI = "Use ANSI Color? [Y/n]: "
CFG_SAY = "`G$n says `g'`G$t`g'`0"
CFG_SAY_SELF = "`GYou say `g'`G$t`g'`0"

# Project root: the parent of the embermud/ package directory.
# This lets paths resolve correctly regardless of where the process is started.
import os as _os
_PROJECT_ROOT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))

CFG_AREA_DIR = _os.path.join(_PROJECT_ROOT, "area", "")
CFG_PLAYER_DIR = _os.path.join(_PROJECT_ROOT, "player", "")
CFG_PLAYER_TEMP = _os.path.join(_PROJECT_ROOT, "player", "temp.tmp")
CFG_LOG_DIR = _os.path.join(_PROJECT_ROOT, "log", "")
CFG_HELP_FILE = "help.txt"
CFG_BUG_FILE = _os.path.join(_PROJECT_ROOT, "log", "bugs.txt")

# Pager
PAGELEN = 22

# Telnet protocol bytes
IAC = 255
WILL = 251
WONT = 252
TELOPT_ECHO = 1
GA = 249

ECHO_OFF_STR = bytes([IAC, WILL, TELOPT_ECHO])
ECHO_ON_STR = bytes([IAC, WONT, TELOPT_ECHO])
GO_AHEAD_STR = bytes([IAC, GA])
