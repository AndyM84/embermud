"""Player character save/load.

Ports save.c from the C version. Maintains the same file format
for compatibility with existing player files.
"""

from __future__ import annotations

import os
import time
from typing import Optional

from . import game
from .area_loader import fread_number, fread_string, fread_word
from .config import (
    MAX_ALIAS, PAGELEN, POS_FIGHTING, POS_STANDING, ROOM_VNUM_TEMPLE,
    SEX_NEUTRAL,
)
from .data import CharData, Descriptor, PcData
from .handler import is_npc
from .log import bug, log_string
from .util import capitalize, str_cmp


def save_char_obj(ch: CharData) -> None:
    """Save a player character to disk."""
    if is_npc(ch):
        return

    strsave = os.path.join(game.player_dir, capitalize(ch.name))
    strtmp = game.player_temp

    try:
        with open(strtmp, "w", encoding="latin-1") as fp:
            fwrite_char(ch, fp)
            fp.write("#END\n")
    except OSError:
        bug("save_char_obj: fopen")
        return

    # Atomically replace the old save file
    try:
        if os.path.exists(strsave):
            os.remove(strsave)
        os.rename(strtmp, strsave)
    except OSError:
        bug("save_char_obj: rename")


def fwrite_char(ch: CharData, fp) -> None:
    """Write all character fields to file."""
    fp.write("#PLAYER\n")
    fp.write(f"Name {ch.name}~\n")
    fp.write(f"ShD  {ch.short_descr}~\n")
    fp.write(f"LnD  {ch.long_descr}~\n")
    fp.write(f"Desc {ch.description}~\n")
    fp.write(f"Sex  {ch.sex}\n")
    fp.write(f"Levl {ch.level}\n")
    fp.write(f"Tru  {ch.trust}\n")
    fp.write(f"Plyd {ch.played + int(game.current_time - ch.logon)}\n")
    fp.write(f"Scro {ch.lines}\n")
    fp.write(f"Room {ch.in_room.vnum if ch.in_room else ROOM_VNUM_TEMPLE}\n")
    fp.write(f"Act  {ch.act}\n")
    fp.write(f"Comm {ch.comm}\n")
    pos = POS_STANDING if ch.position == POS_FIGHTING else ch.position
    fp.write(f"Pos  {pos}\n")
    fp.write(f"InvL {ch.invis_level}\n")

    if ch.pcdata is not None:
        fp.write(f"Pass {ch.pcdata.pwd}~\n")
        fp.write(f"Titl {ch.pcdata.title}~\n")
        fp.write(f"Prom {ch.pcdata.prompt}~\n")

        for alias, sub in ch.pcdata.aliases:
            if alias and sub:
                fp.write(f"Alias {alias}~ {sub}~\n")

    fp.write("End\n\n")


def load_char_obj(d: Descriptor, name: str) -> bool:
    """Load a player character from disk.

    Returns True if an existing file was found.
    """
    ch = CharData(
        name=name,
        short_descr="",
        long_descr="",
        description="",
        level=0,
        trust=0,
        lines=PAGELEN,
        position=POS_STANDING,
        logon=game.current_time,
        played=0,
        act=0,
        comm=0,
        invis_level=0,
        sex=SEX_NEUTRAL,
    )

    pcdata = PcData(
        pwd="",
        title="",
        prompt="> ",
    )
    ch.pcdata = pcdata

    d.character = ch
    ch.desc = d

    strsave = os.path.join(game.player_dir, capitalize(name))
    found = False

    try:
        with open(strsave, "r", encoding="latin-1") as fp:
            while True:
                c = fp.read(1)
                if not c:
                    break
                if c == '#':
                    word = fread_word(fp)
                    if not str_cmp(word, "PLAYER"):
                        fread_char(ch, fp)
                        found = True
                    elif not str_cmp(word, "END"):
                        break
    except OSError:
        pass

    return found


def fread_char(ch: CharData, fp) -> None:
    """Read character data from a player file."""
    while True:
        word = fread_word(fp)
        if not word:
            bug("fread_char: EOF without End marker.")
            return

        if not str_cmp(word, "End"):
            return

        matched = False
        key = word[0].upper() if word else ''

        if key == 'A':
            if not str_cmp(word, "Act"):
                ch.act = fread_number(fp)
                matched = True
            elif not str_cmp(word, "Alias"):
                alias = fread_string(fp)
                sub = fread_string(fp)
                if ch.pcdata and len(ch.pcdata.aliases) < MAX_ALIAS:
                    ch.pcdata.aliases.append((alias, sub))
                matched = True

        elif key == 'C':
            if not str_cmp(word, "Comm"):
                ch.comm = fread_number(fp)
                matched = True

        elif key == 'D':
            if not str_cmp(word, "Desc"):
                ch.description = fread_string(fp)
                matched = True

        elif key == 'I':
            if not str_cmp(word, "InvL"):
                ch.invis_level = fread_number(fp)
                matched = True

        elif key == 'L':
            if not str_cmp(word, "Levl"):
                ch.level = fread_number(fp)
                matched = True
            elif not str_cmp(word, "LnD"):
                ch.long_descr = fread_string(fp)
                matched = True

        elif key == 'N':
            if not str_cmp(word, "Name"):
                ch.name = fread_string(fp)
                matched = True

        elif key == 'P':
            if not str_cmp(word, "Pass"):
                if ch.pcdata:
                    ch.pcdata.pwd = fread_string(fp)
                matched = True
            elif not str_cmp(word, "Plyd"):
                ch.played = fread_number(fp)
                matched = True
            elif not str_cmp(word, "Pos"):
                ch.position = fread_number(fp)
                matched = True
            elif not str_cmp(word, "Prom"):
                if ch.pcdata:
                    ch.pcdata.prompt = fread_string(fp)
                matched = True

        elif key == 'R':
            if not str_cmp(word, "Room"):
                vnum = fread_number(fp)
                room = game.get_room_index(vnum)
                if room is None:
                    room = game.get_room_index(ROOM_VNUM_TEMPLE)
                ch.in_room = room
                matched = True

        elif key == 'S':
            if not str_cmp(word, "Sex"):
                ch.sex = fread_number(fp)
                matched = True
            elif not str_cmp(word, "ShD"):
                ch.short_descr = fread_string(fp)
                matched = True
            elif not str_cmp(word, "Scro"):
                ch.lines = fread_number(fp)
                matched = True

        elif key == 'T':
            if not str_cmp(word, "Tru"):
                ch.trust = fread_number(fp)
                matched = True
            elif not str_cmp(word, "Titl"):
                if ch.pcdata:
                    ch.pcdata.title = fread_string(fp)
                matched = True

        if not matched:
            bug("fread_char: no match for keyword.")
            # Skip to end of line
            while True:
                c = fp.read(1)
                if not c or c in ('\n', '\r'):
                    break
