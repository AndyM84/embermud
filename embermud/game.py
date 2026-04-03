"""Global game state singleton.

All mutable game state lives here so modules can import and share it
without circular dependency issues.
"""

from __future__ import annotations

import time
from typing import Optional

from .data import BanData, CharData, Descriptor, HelpData, RoomIndex


# Character/player lists
char_list: list[CharData] = []
player_list: list[CharData] = []

# Descriptors (active connections)
descriptor_list: list[Descriptor] = []

# Room index (vnum -> RoomIndex)
room_index: dict[int, RoomIndex] = {}

# Help system
help_entries: list[HelpData] = []
help_greeting: Optional[str] = None

# Ban list
ban_list: list[BanData] = []

# Game state flags
merc_down: bool = False
wizlock: bool = False
newlock: bool = False
f_boot_db: bool = False

# Current time (updated each pulse)
current_time: float = time.time()
boot_time: str = ""

# Directory paths (set during boot)
area_dir: str = ""
player_dir: str = ""
player_temp: str = ""
log_dir: str = ""


def get_room_index(vnum: int) -> Optional[RoomIndex]:
    """Look up a room by vnum."""
    return room_index.get(vnum)
