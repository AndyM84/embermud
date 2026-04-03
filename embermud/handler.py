"""Character and room manipulation functions.

Ports handler.c from the C version.
"""

from __future__ import annotations

import time
from typing import Optional

from . import game
from .config import (
    LEVEL_IMMORTAL, PLR_IS_NPC, ROOM_DARK, ROOM_PRIVATE, ROOM_SOLITARY,
    ROOM_VNUM_TEMPLE,
)
from .data import CharData, RoomIndex
from .log import bug
from .util import is_name, number_argument


def get_trust(ch: CharData) -> int:
    """Get a character's effective trust level."""
    if ch.trust != 0:
        return ch.trust
    return ch.level


def is_npc(ch: CharData) -> bool:
    """Check if character is an NPC."""
    return bool(ch.act & PLR_IS_NPC)


def is_immortal(ch: CharData) -> bool:
    """Check if character is immortal level."""
    return get_trust(ch) >= LEVEL_IMMORTAL


def is_awake(ch: CharData) -> bool:
    """Check if character is awake."""
    from .config import POS_SLEEPING
    return ch.position > POS_SLEEPING


def char_from_room(ch: CharData) -> None:
    """Remove a character from their current room."""
    if ch.in_room is None:
        bug("char_from_room: NULL.")
        return

    if ch in ch.in_room.people:
        ch.in_room.people.remove(ch)

    ch.in_room = None


def char_to_room(ch: CharData, room: Optional[RoomIndex]) -> None:
    """Place a character into a room."""
    if room is None:
        bug("char_to_room: NULL room.")
        room = game.get_room_index(ROOM_VNUM_TEMPLE)
        if room is None:
            bug("char_to_room: ROOM_VNUM_TEMPLE does not exist.")
            return

    ch.in_room = room
    room.people.insert(0, ch)


def extract_char(ch: CharData, f_pull: bool) -> None:
    """Remove a character from the game."""
    if ch is None:
        bug("extract_char: NULL ch.")
        return

    if ch.in_room is not None:
        char_from_room(ch)

    if not f_pull:
        return

    if ch in game.char_list:
        game.char_list.remove(ch)

    if not is_npc(ch) and ch in game.player_list:
        game.player_list.remove(ch)

    if ch.desc is not None:
        ch.desc.character = None

    ch.desc = None


def get_char_room(ch: CharData, argument: str) -> Optional[CharData]:
    """Find a character in the same room by name."""
    if ch is None or ch.in_room is None:
        return None

    number, arg = number_argument(argument)
    if not arg:
        return None

    count = 0
    for rch in ch.in_room.people:
        if not can_see(ch, rch):
            continue
        if not is_name(arg, rch.name):
            continue
        count += 1
        if count == number:
            return rch

    return None


def get_player_world(ch: CharData, argument: str) -> Optional[CharData]:
    """Find a player anywhere in the world by name."""
    number, arg = number_argument(argument)
    if not arg:
        return None

    count = 0
    for wch in game.player_list:
        if not can_see(ch, wch):
            continue
        if not is_name(arg, wch.name):
            continue
        count += 1
        if count == number:
            return wch

    return None


def can_see(ch: CharData, victim: CharData) -> bool:
    """Simplified visibility check."""
    if ch is victim:
        return True
    if victim.invis_level > get_trust(ch):
        return False
    return True


def can_see_room(ch: CharData, room: RoomIndex) -> bool:
    """Simplified room visibility check."""
    return True


def room_is_dark(room: Optional[RoomIndex]) -> bool:
    """Check if a room is dark."""
    if room is None:
        return False
    return bool(room.room_flags & ROOM_DARK) and room.light < 1


def room_is_private(room: Optional[RoomIndex]) -> bool:
    """Check if a room is private (occupancy limit)."""
    if room is None:
        return False

    count = len(room.people)

    if (room.room_flags & ROOM_PRIVATE) and count >= 2:
        return True
    if (room.room_flags & ROOM_SOLITARY) and count >= 1:
        return True

    return False


def get_curtime() -> str:
    """Return current time as a string."""
    return time.strftime("%c")


def get_curdate() -> str:
    """Return current date in mm/dd/yy format."""
    return time.strftime("%m/%d/%y")
