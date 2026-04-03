"""Core data structures for EmberMUD."""

from __future__ import annotations

import time
from dataclasses import dataclass, field
from typing import Optional

from .config import (
    MAX_ALIAS, MAX_DIR, POS_STANDING, SEX_NEUTRAL, PAGELEN,
)


@dataclass
class ExitData:
    """An exit from a room."""
    to_room: Optional[RoomIndex] = None
    to_vnum: int = 0  # Used during loading before resolution
    exit_info: int = 0
    key: int = 0
    keyword: str = ""
    description: str = ""


@dataclass
class HelpData:
    """A help entry."""
    level: int = 0
    keyword: str = ""
    text: str = ""


@dataclass
class BanData:
    """A site ban entry."""
    name: str = ""
    ban_flags: int = 0
    level: int = 0


@dataclass
class PcData:
    """Player-specific data (not for NPCs)."""
    pwd: str = ""
    title: str = ""
    prompt: str = "> "
    aliases: list[tuple[str, str]] = field(default_factory=list)
    confirm_delete: bool = False


@dataclass
class CharData:
    """A character (player or NPC)."""
    name: str = ""
    short_descr: str = ""
    long_descr: str = ""
    description: str = ""
    sex: int = SEX_NEUTRAL
    level: int = 0
    trust: int = 0
    played: int = 0
    lines: int = PAGELEN
    logon: float = field(default_factory=time.time)
    timer: int = 0
    wait: int = 0
    act: int = 0
    comm: int = 0
    invis_level: int = 0
    position: int = POS_STANDING

    # Relationships
    desc: Optional[Descriptor] = field(default=None, repr=False)
    in_room: Optional[RoomIndex] = field(default=None, repr=False)
    was_in_room: Optional[RoomIndex] = field(default=None, repr=False)
    pcdata: Optional[PcData] = field(default=None, repr=False)


@dataclass
class RoomIndex:
    """A room in the world."""
    vnum: int = 0
    name: str = ""
    description: str = ""
    room_flags: int = 0
    light: int = 0
    sector_type: int = 0
    exits: list[Optional[ExitData]] = field(default_factory=lambda: [None] * MAX_DIR)
    people: list[CharData] = field(default_factory=list, repr=False)


@dataclass
class Descriptor:
    """A network connection."""
    reader: object = field(default=None, repr=False)  # asyncio.StreamReader
    writer: object = field(default=None, repr=False)  # asyncio.StreamWriter
    host: str = ""
    connected: int = 0
    fcommand: bool = False
    ansi: bool = True

    inbuf: str = ""
    incomm: str = ""
    inlast: str = ""
    repeat: int = 0
    outbuf: str = ""

    showstr_head: str = ""
    showstr_point: int = 0

    character: Optional[CharData] = field(default=None, repr=False)
