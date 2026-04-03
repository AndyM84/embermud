"""Database initialization and world bootstrapping.

Ports boot_db() and create_minimal_world() from db.c.
"""

from __future__ import annotations

import random

from . import game
from .area_loader import fix_exits, load_area_files
from .config import (
    CFG_AREA_DIR, CFG_BUG_FILE, CFG_HELP_FILE, CFG_LOG_DIR,
    CFG_PLAYER_DIR, CFG_PLAYER_TEMP, DIR_DOWN, DIR_EAST, DIR_NORTH,
    DIR_SOUTH, DIR_UP, DIR_WEST, ROOM_DARK, ROOM_INDOORS, ROOM_NO_MOB,
    ROOM_SAFE, ROOM_VNUM_LIMBO, ROOM_VNUM_TEMPLE, SECT_CITY, SECT_FIELD,
    SECT_INSIDE,
)
from .data import ExitData, HelpData, RoomIndex
from .log import log_string, logf_string


def create_minimal_world() -> None:
    """Create a minimal hardcoded world with 8 rooms and connecting exits."""

    def make_room(vnum, name, desc, flags=0, sector=SECT_INSIDE):
        room = RoomIndex(
            vnum=vnum, name=name, description=desc,
            room_flags=flags, sector_type=sector,
        )
        game.room_index[vnum] = room
        return room

    def make_exit(from_room, direction, to_room):
        ex = ExitData(to_room=to_room)
        from_room.exits[direction] = ex

    limbo = make_room(ROOM_VNUM_LIMBO, "Limbo",
        "You are floating in an endless void of grey nothingness.\r\n"
        "There is no ground beneath your feet, no sky above your head.\r\n"
        "The silence here is absolute and oppressive, broken only by\r\n"
        "the faint echo of your own thoughts.\r\n",
        flags=ROOM_SAFE)

    square = make_room(ROOM_VNUM_TEMPLE, "Town Square",
        "You stand at the heart of a small medieval town. Cobblestones\r\n"
        "stretch in every direction, worn smooth by centuries of foot\r\n"
        "traffic. A weathered stone fountain sits in the center of the\r\n"
        "square, its basin filled with clear water. Roads lead north,\r\n"
        "south, east, and west, while a spiral staircase leads up to a\r\n"
        "watchtower and a trapdoor opens into a cellar below.\r\n",
        flags=ROOM_SAFE | ROOM_NO_MOB, sector=SECT_CITY)

    north_road = make_room(3002, "North Road",
        "A hard-packed dirt road stretches northward into rolling green\r\n"
        "hills. Wooden fences line both sides of the path, enclosing\r\n"
        "small pastures where sheep graze lazily. The town square lies\r\n"
        "to the south, its fountain just visible over the rooftops.\r\n",
        sector=SECT_FIELD)

    south_gate = make_room(3003, "South Gate",
        "A massive wooden gate reinforced with iron bands marks the\r\n"
        "southern entrance to the town. Two stone pillars flank the\r\n"
        "gateway, each topped with a flickering torch. Beyond the\r\n"
        "gate, a dusty road winds away into dark, dense forest.\r\n",
        sector=SECT_CITY)

    east_market = make_room(3004, "East Market",
        "Rows of wooden stalls and canvas-covered booths fill this\r\n"
        "bustling marketplace. The air carries the mingled scents of\r\n"
        "fresh bread, spiced meats, and exotic herbs. Colorful banners\r\n"
        "hang between the stalls, snapping gently in the breeze.\r\n",
        sector=SECT_CITY)

    west_garden = make_room(3005, "West Garden",
        "A tranquil garden occupies this corner of town, enclosed by\r\n"
        "a low stone wall covered in climbing ivy. Ancient oak trees\r\n"
        "provide dappled shade over beds of wildflowers and medicinal\r\n"
        "herbs. A moss-covered stone bench invites quiet reflection.\r\n",
        flags=ROOM_SAFE, sector=SECT_FIELD)

    watch_tower = make_room(3006, "Watch Tower",
        "You stand atop a tall stone watchtower that rises above the\r\n"
        "town rooftops. From this vantage point, the entire settlement\r\n"
        "spreads below you -- the square, the market, the garden, and\r\n"
        "the roads leading away into the countryside. A cold wind whips\r\n"
        "at your clothing. A spiral staircase leads back down.\r\n",
        flags=ROOM_INDOORS)

    cellar = make_room(3007, "Cellar",
        "A damp, cool cellar stretches beneath the town square. Thick\r\n"
        "stone walls are lined with wooden shelves holding dusty bottles\r\n"
        "and forgotten provisions. Cobwebs drape from the low ceiling,\r\n"
        "and the only light filters down from the trapdoor above.\r\n",
        flags=ROOM_DARK | ROOM_INDOORS)

    # Create exits
    make_exit(square, DIR_NORTH, north_road)
    make_exit(square, DIR_SOUTH, south_gate)
    make_exit(square, DIR_EAST, east_market)
    make_exit(square, DIR_WEST, west_garden)
    make_exit(square, DIR_UP, watch_tower)
    make_exit(square, DIR_DOWN, cellar)

    make_exit(north_road, DIR_SOUTH, square)
    make_exit(south_gate, DIR_NORTH, square)
    make_exit(east_market, DIR_WEST, square)
    make_exit(west_garden, DIR_EAST, square)
    make_exit(watch_tower, DIR_DOWN, square)
    make_exit(cellar, DIR_UP, square)

    log_string("Minimal world created: 8 rooms, 12 exits.")


def create_basic_helps() -> None:
    """Create built-in help entries."""
    game.help_entries.append(HelpData(
        level=0, keyword="MOTD",
        text="Welcome back!  This MUD is currently in development.\r\n"
             "Please report any bugs you find.\r\n"))

    game.help_entries.append(HelpData(
        level=0, keyword="RULES",
        text="1. Be respectful to other players.\r\n"
             "2. No harassment or abuse.\r\n"
             "3. Have fun!\r\n"))

    game.help_entries.append(HelpData(
        level=0, keyword="COMMANDS SUMMARY",
        text="`WAvailable Commands:`0\r\n"
             "  look        - Look at your surroundings\r\n"
             "  exits       - Show available exits\r\n"
             "  north south east west up down - Move in a direction\r\n"
             "  say <msg>   - Say something to the room\r\n"
             "  tell <who> <msg> - Send a private message\r\n"
             "  who         - List online players\r\n"
             "  save        - Save your character\r\n"
             "  quit        - Save and leave the game\r\n"
             "  help <topic>- Get help on a topic\r\n"))


def boot_db() -> None:
    """Initialize the game database."""
    game.f_boot_db = True

    random.seed()

    # Set directory paths
    game.area_dir = CFG_AREA_DIR
    game.player_dir = CFG_PLAYER_DIR
    game.player_temp = CFG_PLAYER_TEMP
    game.log_dir = CFG_LOG_DIR

    # Try loading area files
    load_area_files()

    # If no rooms loaded, use hardcoded fallback
    if (game.get_room_index(ROOM_VNUM_LIMBO) is None and
            game.get_room_index(ROOM_VNUM_TEMPLE) is None):
        log_string("No rooms loaded from area files, using hardcoded world.")
        create_minimal_world()
    else:
        fix_exits()

    # If no help entries, create basic ones
    if not game.help_entries:
        log_string("No help entries loaded from area files, using built-in helps.")
        create_basic_helps()

    # Set default greeting if none loaded
    if game.help_greeting is None:
        game.help_greeting = (
            "\r\n"
            "`WWelcome to EmberMUD!`w\r\n"
            "\r\n"
            "Based on ROM 2.4, Merc 2.1, and DikuMUD.\r\n"
            "Type your character name to begin.\r\n"
            "\r\n"
        )

    # Log summary
    n_rooms = len(game.room_index)
    n_exits = sum(
        1 for room in game.room_index.values()
        for ex in room.exits if ex is not None
    )
    n_helps = len(game.help_entries)
    logf_string("Boot complete: %d rooms, %d exits, %d help entries.",
                n_rooms, n_exits, n_helps)

    game.f_boot_db = False
