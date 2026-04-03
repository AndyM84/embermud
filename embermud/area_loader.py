"""Area file loader - reads MUD area files in the standard ROM/Merc format.

Ports the file I/O and area loading from db.c.
"""

from __future__ import annotations

import os
from typing import Optional, TextIO

from . import game
from .config import (
    EX_CLOSED, EX_ISDOOR, EX_LOCKED, EX_PICKPROOF,
)
from .data import ExitData, HelpData, RoomIndex
from .log import bug, log_string, logf_string
from .util import str_cmp


def fread_letter(fp: TextIO) -> str:
    """Read a non-whitespace character from a file."""
    while True:
        c = fp.read(1)
        if not c:
            return ''
        if not c.isspace():
            return c


def fread_number(fp: TextIO) -> int:
    """Read a number from a file, handling +/- signs and | chaining."""
    # Skip whitespace
    c = ''
    while True:
        c = fp.read(1)
        if not c:
            return 0
        if not c.isspace():
            break

    sign = 1
    if c == '+':
        c = fp.read(1)
    elif c == '-':
        sign = -1
        c = fp.read(1)

    if not c or not c.isdigit():
        bug("fread_number: bad format.")
        return 0

    number = 0
    while c and c.isdigit():
        number = number * 10 + int(c)
        c = fp.read(1)

    number *= sign

    if c == '|':
        number += fread_number(fp)
    elif c and c != ' ':
        # Put back the character by seeking back
        fp.seek(fp.tell() - 1)

    return number


def fread_flag(fp: TextIO) -> int:
    """Read a flag value, supporting alpha encoding."""
    c = ''
    while True:
        c = fp.read(1)
        if not c:
            return 0
        if not c.isspace():
            break

    number = 0

    if not c.isdigit() and c != '-':
        # Alpha flag encoding
        while c.isalpha():
            if 'A' <= c <= 'Z':
                bitsum = 1 << (ord(c) - ord('A'))
            elif 'a' <= c <= 'z':
                bitsum = 1 << (26 + ord(c) - ord('a'))
            else:
                bitsum = 0
            number += bitsum
            c = fp.read(1)
            if not c:
                return number

    if c == '-':
        return -fread_number(fp)

    while c and c.isdigit():
        number = number * 10 + int(c)
        c = fp.read(1)

    if c == '|':
        number += fread_flag(fp)
    elif c and c != ' ':
        fp.seek(fp.tell() - 1)

    return number


def fread_string(fp: TextIO) -> str:
    """Read a tilde-terminated string from a file."""
    # Skip leading whitespace
    c = ''
    while True:
        c = fp.read(1)
        if not c:
            return ""
        if not c.isspace():
            break

    if c == '~':
        return ""

    buf = [c]
    while True:
        c = fp.read(1)
        if not c:
            bug("fread_string: EOF")
            return "".join(buf)

        if c == '~':
            return "".join(buf)
        elif c == '\n':
            buf.append('\r')
            buf.append('\n')
        elif c == '\r':
            pass  # Skip carriage returns
        else:
            buf.append(c)


def fread_to_eol(fp: TextIO) -> None:
    """Read to end of line."""
    while True:
        c = fp.read(1)
        if not c or c in ('\n', '\r'):
            break

    # Skip additional newline characters
    while True:
        c = fp.read(1)
        if not c:
            return
        if c not in ('\n', '\r'):
            fp.seek(fp.tell() - 1)
            return


def fread_word(fp: TextIO) -> str:
    """Read one word from a file."""
    # Skip whitespace
    c = ''
    while True:
        c = fp.read(1)
        if not c:
            return ""
        if not c.isspace():
            break

    if c in ("'", '"'):
        end_char = c
        word = []
    else:
        end_char = ' '
        word = [c]

    while True:
        c = fp.read(1)
        if not c:
            break
        if end_char == ' ':
            if c.isspace():
                fp.seek(fp.tell() - 1)
                break
        elif c == end_char:
            break
        word.append(c)

    return "".join(word)


def load_rooms(fp: TextIO) -> None:
    """Load #ROOMS section from an area file."""
    while True:
        letter = fread_letter(fp)
        if letter != '#':
            bug("load_rooms: # not found.")
            return

        vnum = fread_number(fp)
        if vnum == 0:
            break

        if vnum in game.room_index:
            bug(f"load_rooms: duplicate vnum {vnum}.")
            return

        room = RoomIndex(vnum=vnum)
        room.name = fread_string(fp)
        room.description = fread_string(fp)
        fread_number(fp)  # area_num (unused)
        room.room_flags = fread_flag(fp)
        room.sector_type = fread_number(fp)
        room.light = 0

        while True:
            letter = fread_letter(fp)

            if letter == 'S':
                break
            elif letter == 'D':
                door = fread_number(fp)
                if door < 0 or door > 5:
                    bug(f"load_rooms: bad door {door} in vnum {vnum}.")
                    return

                pexit = ExitData()
                pexit.description = fread_string(fp)
                pexit.keyword = fread_string(fp)
                locks = fread_number(fp)
                pexit.key = fread_number(fp)
                pexit.to_vnum = fread_number(fp)

                if locks == 0:
                    pexit.exit_info = 0
                elif locks == 1:
                    pexit.exit_info = EX_ISDOOR
                elif locks == 2:
                    pexit.exit_info = EX_ISDOOR | EX_CLOSED | EX_LOCKED | EX_PICKPROOF
                else:
                    pexit.exit_info = EX_ISDOOR | EX_CLOSED | EX_LOCKED

                room.exits[door] = pexit
            elif letter == 'E':
                # Extra description - read and discard
                fread_string(fp)
                fread_string(fp)
            else:
                bug(f"load_rooms: unknown sub-record '{letter}' in vnum {vnum}.")
                return

        game.room_index[vnum] = room


def load_helps(fp: TextIO) -> None:
    """Load #HELPS section from an area file."""
    while True:
        level = fread_number(fp)
        keyword = fread_string(fp)

        if keyword.startswith('$'):
            break

        help_entry = HelpData(level=level, keyword=keyword, text=fread_string(fp))

        game.help_entries.append(help_entry)

        if not str_cmp(keyword, "GREETING") or not str_cmp(keyword, "ANSIGREET"):
            game.help_greeting = help_entry.text


def fix_exits() -> None:
    """After all rooms are loaded, resolve exit vnums to room references."""
    unresolved = 0

    for room in game.room_index.values():
        for door in range(6):
            pexit = room.exits[door]
            if pexit is not None:
                if pexit.to_vnum <= 0:
                    pexit.to_room = None
                else:
                    pexit.to_room = game.get_room_index(pexit.to_vnum)
                    if pexit.to_room is None:
                        logf_string(
                            "fix_exits: room %d exit %d -> vnum %d not found.",
                            room.vnum, door, pexit.to_vnum
                        )
                        unresolved += 1

    if unresolved > 0:
        logf_string("fix_exits: %d unresolved exit(s).", unresolved)


def skip_section_areadata(fp: TextIO) -> None:
    """Skip #AREADATA section."""
    while True:
        word = fread_word(fp)
        if not str_cmp(word, "End"):
            break
        fread_to_eol(fp)


def skip_section_vnum_list(fp: TextIO) -> None:
    """Skip #MOBILES or #OBJECTS section (scan for #0)."""
    at_hash = False
    while True:
        c = fp.read(1)
        if not c:
            break

        if c == '#':
            at_hash = True
            continue

        if at_hash and c == '0':
            nxt = fp.read(1)
            if not nxt or nxt.isspace():
                if nxt:
                    fp.seek(fp.tell() - 1)
                break

        at_hash = False


def skip_section_word_until_s(fp: TextIO) -> None:
    """Skip section reading words until lone 'S'."""
    while True:
        word = fread_word(fp)
        if word == 'S':
            break
        fread_to_eol(fp)


def skip_section_shops(fp: TextIO) -> None:
    """Skip #SHOPS section."""
    while True:
        keeper = fread_number(fp)
        if keeper == 0:
            break
        fread_to_eol(fp)


def skip_section_unknown(fp: TextIO) -> None:
    """Skip an unknown section."""
    at_line_start = True
    while True:
        c = fp.read(1)
        if not c:
            break

        if at_line_start and c == '#':
            nxt = fp.read(1)
            if not nxt:
                break
            if nxt.isupper() or nxt == '$':
                fp.seek(fp.tell() - 1)
                fp.seek(fp.tell() - 1)
                break

        at_line_start = c in ('\n', '\r')


def load_area_file(filename: str) -> None:
    """Load a single area file, dispatching each section."""
    try:
        fp = open(filename, "r", encoding="latin-1")
    except OSError:
        logf_string("load_area_file: could not open %s, skipping.", filename)
        return

    try:
        while True:
            letter = fread_letter(fp)
            if not letter or letter != '#':
                break

            word = fread_word(fp)
            if word.startswith('$'):
                break
            elif not str_cmp(word, "AREADATA"):
                skip_section_areadata(fp)
            elif not str_cmp(word, "HELPS"):
                load_helps(fp)
            elif not str_cmp(word, "MOBILES"):
                skip_section_vnum_list(fp)
            elif not str_cmp(word, "OBJECTS"):
                skip_section_vnum_list(fp)
            elif not str_cmp(word, "ROOMS"):
                load_rooms(fp)
            elif not str_cmp(word, "RESETS"):
                skip_section_word_until_s(fp)
            elif not str_cmp(word, "SHOPS"):
                skip_section_shops(fp)
            elif not str_cmp(word, "PROGS"):
                skip_section_word_until_s(fp)
            else:
                logf_string("load_area_file: unknown section '%s' in %s, skipping.", word, filename)
                skip_section_unknown(fp)
    finally:
        fp.close()


def load_area_files() -> None:
    """Load all area files listed in area.lst."""
    lst_path = os.path.join(game.area_dir, "area.lst")

    try:
        fp = open(lst_path, "r", encoding="latin-1")
    except OSError:
        logf_string("load_area_files: %s not found, using hardcoded world.", lst_path)
        return

    try:
        while True:
            word = fread_word(fp)
            if not word or word.startswith('$'):
                break
            filename = os.path.join(game.area_dir, word)
            load_area_file(filename)
    finally:
        fp.close()
