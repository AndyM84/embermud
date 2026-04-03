"""Basic player commands: look, movement, say, tell, who, quit, save, help.

Ports act_basic.c from the C version.
"""

from __future__ import annotations

from . import game
from .config import (
    CFG_QUIT, CFG_SAY, CFG_SAY_SELF, CON_PLAYING, COMM_BRIEF, DIR_DOWN,
    DIR_EAST, DIR_NORTH, DIR_SOUTH, DIR_UP, DIR_WEST, EX_CLOSED, MAX_ALIAS,
    MAX_DIR, MAX_STRING_LENGTH, PLR_AUTOEXIT, PLR_COLOUR, PLR_IS_NPC,
    POS_DEAD, POS_FIGHTING, POS_RESTING, POS_SLEEPING, POS_STANDING,
    dir_name,
)
from .data import CharData, Descriptor
from .handler import (
    can_see, can_see_room, char_from_room, char_to_room, extract_char,
    get_player_world, get_trust, is_npc, room_is_private,
)
from .log import bug, log_string
from .save import save_char_obj
from .util import capitalize, is_name, one_argument, str_cmp, str_prefix


# ---------------------------------------------------------------------------
# Output helpers (these need to be importable by other modules too)
# ---------------------------------------------------------------------------

def send_to_char(txt: str, ch: CharData) -> None:
    """Send text to a character."""
    if txt and ch.desc is not None:
        ch.desc.outbuf += txt


def printf_to_char(ch: CharData, fmt: str, *args) -> None:
    """Send formatted text to a character."""
    send_to_char(fmt % args if args else fmt, ch)


def send_to_room(txt: str, room) -> None:
    """Send text to all characters in a room."""
    if not txt or room is None:
        return
    for ch in room.people:
        if ch.desc is not None:
            send_to_char(txt, ch)


def act_string(fmt: str, to: CharData, ch: CharData, arg1, arg2) -> str:
    """Process act() format codes and return the result string."""
    he_she = ["it", "he", "she"]
    him_her = ["it", "him", "her"]
    his_her = ["its", "his", "her"]

    vch = arg2 if isinstance(arg2, CharData) else None

    out = []
    i = 0
    while i < len(fmt):
        if fmt[i] != '$':
            out.append(fmt[i])
            i += 1
            continue

        i += 1
        if i >= len(fmt):
            break

        code = fmt[i]
        i += 1

        if code in ('A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
                     'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                     'W', 'X', 'Y', 'Z') and arg2 is None:
            replacement = " <@@@> "
        else:
            if code == 't':
                replacement = str(arg1) if arg1 else ""
            elif code == 'T':
                replacement = str(arg2) if arg2 else ""
            elif code == 'n':
                replacement = ch.name
            elif code == 'N':
                replacement = vch.name if vch else ""
            elif code == 'e':
                replacement = he_she[min(max(ch.sex, 0), 2)]
            elif code == 'E':
                replacement = he_she[min(max(vch.sex, 0), 2)] if vch else "it"
            elif code == 'm':
                replacement = him_her[min(max(ch.sex, 0), 2)]
            elif code == 'M':
                replacement = him_her[min(max(vch.sex, 0), 2)] if vch else "it"
            elif code == 's':
                replacement = his_her[min(max(ch.sex, 0), 2)]
            elif code == 'S':
                replacement = his_her[min(max(vch.sex, 0), 2)] if vch else "its"
            elif code == 'd':
                if not arg2 or (isinstance(arg2, str) and not arg2):
                    replacement = "door"
                else:
                    _, replacement = one_argument(str(arg2))
            else:
                replacement = " <@@@> "

        out.append(replacement)

    result = "".join(out) + "`w\r\n"
    if result:
        result = result[0].upper() + result[1:]
    return result


def act(fmt: str, ch: CharData, arg1, arg2, act_type: int = 0,
        min_pos: int = POS_RESTING, **kwargs) -> None:
    """Send formatted message to appropriate targets."""
    from .config import TO_CHAR, TO_ROOM, TO_VICT, TO_NOTVICT

    if not fmt or not ch:
        return

    vch = arg2 if isinstance(arg2, CharData) else None

    if act_type == TO_CHAR:
        targets = [ch]
    elif act_type == TO_VICT:
        if vch is None or vch.in_room is None:
            return
        targets = [vch]
    elif ch.in_room is not None:
        targets = list(ch.in_room.people)
    else:
        targets = []

    for to in targets:
        if to.desc is None or to.position < min_pos:
            continue
        if act_type == TO_CHAR and to is not ch:
            continue
        if act_type == TO_VICT and (to is not vch or to is ch):
            continue
        if act_type == TO_ROOM and to is ch:
            continue
        if act_type == TO_NOTVICT and (to is ch or to is vch):
            continue

        txt = act_string(fmt, to, ch, arg1, arg2)
        if to.desc:
            to.desc.outbuf += txt


def act_new(fmt: str, ch: CharData, arg1, arg2, act_type: int,
            min_pos: int) -> None:
    """Send formatted message with explicit minimum position."""
    act(fmt, ch, arg1, arg2, act_type=act_type, min_pos=min_pos)


# ---------------------------------------------------------------------------
# Alias substitution
# ---------------------------------------------------------------------------

def substitute_alias(d: Descriptor, input_str: str) -> None:
    """Check for alias expansion before interpreting."""
    from .interp import interpret

    ch = d.character
    if ch is None or is_npc(ch) or ch.pcdata is None:
        if ch is not None:
            interpret(ch, input_str)
        return

    _, arg = one_argument(input_str)

    for alias, alias_sub in ch.pcdata.aliases:
        if not str_cmp(arg, alias):
            # Skip past the first word in original input
            rest = input_str.lstrip()
            # Skip first word
            while rest and not rest[0].isspace():
                rest = rest[1:]
            rest = rest.lstrip()

            buf = f"{alias_sub} {rest}"
            if len(buf) > MAX_STRING_LENGTH - 1:
                send_to_char("Alias substitution too long.\r\n", ch)
                return

            interpret(ch, buf)
            return

    interpret(ch, input_str)


def set_title(ch: CharData, title: str) -> None:
    """Set a player's title string."""
    if is_npc(ch) or ch.pcdata is None:
        return

    if title and title[0] != ' ':
        title = ' ' + title

    ch.pcdata.title = title


def check_social(ch: CharData, command: str, argument: str) -> bool:
    """Stub for social command lookup."""
    return False


# ---------------------------------------------------------------------------
# Informational commands
# ---------------------------------------------------------------------------

def show_exits_to_char(ch: CharData) -> None:
    """Display auto-exit line."""
    buf = "`C[Exits:"
    found = False

    for door in range(MAX_DIR):
        pexit = ch.in_room.exits[door]
        if pexit is not None and pexit.to_room is not None and can_see_room(ch, pexit.to_room):
            found = True
            if pexit.exit_info & EX_CLOSED:
                buf += f" ({dir_name[door]})"
            else:
                buf += f" {dir_name[door]}"

    if not found:
        buf += " none"

    buf += "]`0\r\n"
    send_to_char(buf, ch)


def show_char_to_char(people: list, ch: CharData) -> None:
    """Show all other characters in the room to ch."""
    for rch in people:
        if rch is ch:
            continue
        if not can_see(ch, rch):
            continue

        if (rch.long_descr and rch.position == POS_STANDING and is_npc(rch)):
            send_to_char(rch.long_descr, ch)
        else:
            pos_str = ""
            if rch.position == POS_RESTING:
                pos_str = " (Resting)"
            elif rch.position == POS_SLEEPING:
                pos_str = " (Sleeping)"
            elif rch.position == POS_FIGHTING:
                pos_str = " (Fighting)"
            send_to_char(f"{rch.name}{pos_str} is here.\r\n", ch)


def do_look(ch: CharData, argument: str) -> None:
    """Look at the room."""
    if ch.in_room is None or ch.desc is None:
        return

    if ch.position < POS_SLEEPING:
        send_to_char("You can't see anything but stars!\r\n", ch)
        return

    if ch.position == POS_SLEEPING:
        send_to_char("You can't see anything, you're sleeping!\r\n", ch)
        return

    if not argument or not str_cmp(argument, "auto"):
        send_to_char(f"`C{ch.in_room.name}`0\r\n", ch)

        if (not argument or not (ch.comm & COMM_BRIEF)):
            if ch.in_room.description:
                send_to_char(ch.in_room.description, ch)

        if ch.act & PLR_AUTOEXIT:
            show_exits_to_char(ch)

        show_char_to_char(ch.in_room.people, ch)


def do_exits(ch: CharData, argument: str) -> None:
    """Show verbose exit listing."""
    if ch.in_room is None:
        return

    found = False
    for door in range(MAX_DIR):
        pexit = ch.in_room.exits[door]
        if pexit is not None and pexit.to_room is not None and can_see_room(ch, pexit.to_room):
            found = True
            if pexit.exit_info & EX_CLOSED:
                printf_to_char(ch, "%-5s - (closed)\r\n", capitalize(dir_name[door]))
            else:
                room_name = pexit.to_room.name if pexit.to_room.name else "(no name)"
                printf_to_char(ch, "%-5s - %s\r\n", capitalize(dir_name[door]), room_name)

    if not found:
        send_to_char("None.\r\n", ch)


# ---------------------------------------------------------------------------
# Communication commands
# ---------------------------------------------------------------------------

def do_say(ch: CharData, argument: str) -> None:
    """Say something to the room."""
    if not argument:
        send_to_char("Say what?\r\n", ch)
        return

    from .config import TO_ROOM, TO_CHAR
    act_new(CFG_SAY, ch, argument, None, TO_ROOM, POS_RESTING)
    act_new(CFG_SAY_SELF, ch, argument, None, TO_CHAR, POS_RESTING)


def do_tell(ch: CharData, argument: str) -> None:
    """Send a private message to another player."""
    rest, arg = one_argument(argument)

    if not arg or not rest:
        send_to_char("Tell whom what?\r\n", ch)
        return

    victim = get_player_world(ch, arg)
    if victim is None:
        send_to_char("They aren't here.\r\n", ch)
        return

    if victim.desc is None and not is_npc(victim):
        from .config import TO_CHAR
        act("$N seems to have misplaced $S link...try again later.",
            ch, None, victim, act_type=TO_CHAR)
        return

    from .config import TO_VICT
    act_new("$n tells you '$t'", ch, rest, victim, TO_VICT, POS_DEAD)
    printf_to_char(ch, "You tell %s '%s'\r\n", victim.name, rest)


# ---------------------------------------------------------------------------
# Who / Help / Save / Quit
# ---------------------------------------------------------------------------

def do_who(ch: CharData, argument: str) -> None:
    """Show list of visible online players."""
    count = 0
    send_to_char("`W---[ Who is Online ]---`0\r\n", ch)

    for d in game.descriptor_list:
        if d.connected != CON_PLAYING:
            continue
        wch = d.character
        if wch is None:
            continue
        if not can_see(ch, wch):
            continue

        count += 1
        title = ""
        if wch.pcdata and wch.pcdata.title:
            title = wch.pcdata.title
        send_to_char(f"[{wch.level:3d}] {wch.name}{title}\r\n", ch)

    printf_to_char(ch, "\r\n%d player%s online.\r\n",
                   count, "" if count == 1 else "s")


def do_quit(ch: CharData, argument: str) -> None:
    """Leave the game."""
    if is_npc(ch):
        return

    if ch.position == POS_FIGHTING:
        send_to_char("No way! You are fighting.\r\n", ch)
        return

    send_to_char(CFG_QUIT, ch)
    from .config import TO_ROOM
    act("$n has left the game.", ch, None, None, act_type=TO_ROOM)
    log_string(f"{ch.name} has quit.")

    save_char_obj(ch)

    d = ch.desc
    extract_char(ch, True)

    if d is not None:
        from .comm import close_socket
        close_socket(d)


def do_save(ch: CharData, argument: str) -> None:
    """Save the character to disk."""
    if is_npc(ch):
        return
    save_char_obj(ch)
    send_to_char("Saved.\r\n", ch)


def do_help(ch: CharData, argument: str) -> None:
    """Display a help topic."""
    if not argument:
        argument = "summary"

    for help_entry in game.help_entries:
        if help_entry.level > get_trust(ch):
            continue
        if is_name(argument, help_entry.keyword):
            text = help_entry.text
            if text and text[0] == '.':
                text = text[1:]
            send_to_char(text, ch)
            return

    send_to_char("No help on that word.\r\n", ch)


# ---------------------------------------------------------------------------
# Movement commands
# ---------------------------------------------------------------------------

def move_char(ch: CharData, door: int) -> None:
    """Move a character in the given direction."""
    if ch.in_room is None:
        return

    if door < 0 or door >= MAX_DIR:
        bug(f"move_char: bad door {door}.")
        return

    pexit = ch.in_room.exits[door]
    if pexit is None or pexit.to_room is None:
        send_to_char("Alas, you cannot go that way.\r\n", ch)
        return

    to_room = pexit.to_room

    if pexit.exit_info & EX_CLOSED:
        from .config import TO_CHAR
        act("The $d is closed.", ch, None, pexit.keyword, act_type=TO_CHAR)
        return

    if room_is_private(to_room):
        send_to_char("That room is private right now.\r\n", ch)
        return

    from .config import TO_ROOM
    act("$n leaves $T.", ch, None, dir_name[door], act_type=TO_ROOM)

    char_from_room(ch)
    char_to_room(ch, to_room)

    act("$n has arrived.", ch, None, None, act_type=TO_ROOM)
    do_look(ch, "auto")


def do_north(ch: CharData, argument: str) -> None:
    move_char(ch, DIR_NORTH)

def do_east(ch: CharData, argument: str) -> None:
    move_char(ch, DIR_EAST)

def do_south(ch: CharData, argument: str) -> None:
    move_char(ch, DIR_SOUTH)

def do_west(ch: CharData, argument: str) -> None:
    move_char(ch, DIR_WEST)

def do_up(ch: CharData, argument: str) -> None:
    move_char(ch, DIR_UP)

def do_down(ch: CharData, argument: str) -> None:
    move_char(ch, DIR_DOWN)
