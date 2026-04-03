"""Site ban system.

Ports ban.c from the C version.
"""

from __future__ import annotations

import os

from . import game
from .area_loader import fread_flag, fread_number, fread_to_eol, fread_word
from .config import BAN_ALL, BAN_PERMANENT, BAN_PREFIX, BAN_SUFFIX
from .data import BanData
from .log import log_string
from .util import capitalize, str_prefix, str_suffix

import os as _os
BAN_FILE = _os.path.join(
    _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__))),
    "area", "ban.txt"
)


def save_bans() -> None:
    """Write permanent bans to disk."""
    found = False
    try:
        with open(BAN_FILE, "w") as fp:
            for ban in game.ban_list:
                if ban.ban_flags & BAN_PERMANENT:
                    found = True
                    fp.write(f"{ban.name:<20s} {ban.level:<2d} {ban.ban_flags}\n")
    except OSError:
        log_string(f"save_bans: could not open {BAN_FILE}")
        return

    if not found:
        try:
            os.unlink(BAN_FILE)
        except OSError:
            pass


def load_bans() -> None:
    """Read bans from disk."""
    try:
        fp = open(BAN_FILE, "r")
    except OSError:
        return

    try:
        while True:
            line = fp.readline()
            if not line:
                break
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) < 3:
                continue
            ban = BanData(
                name=parts[0],
                level=int(parts[1]),
                ban_flags=int(parts[2]),
            )
            game.ban_list.append(ban)
    finally:
        fp.close()


def check_ban(host: str) -> bool:
    """Check if a host is banned. Returns True if banned."""
    if not host:
        return False

    for ban in game.ban_list:
        if not (ban.ban_flags & BAN_ALL):
            continue

        if (ban.ban_flags & BAN_PREFIX) and (ban.ban_flags & BAN_SUFFIX):
            if ban.name.lower() in host.lower():
                return True

        if ban.ban_flags & BAN_PREFIX:
            if not str_suffix(ban.name, host):
                return True

        if ban.ban_flags & BAN_SUFFIX:
            if not str_prefix(ban.name, host):
                return True

    return False
