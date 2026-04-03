"""Networking, I/O, game loop, and connection management.

Ports comm.c from the C version, using asyncio instead of select().
"""

from __future__ import annotations

import asyncio
import hashlib
import os
import time

from . import game
from .act_basic import (
    act, act_new, do_help, do_look, send_to_char, set_title,
    substitute_alias,
)
from .ban import check_ban
from .color import do_color
from .config import (
    CFG_ASK_ANSI, CFG_CONNECT_MSG, CON_BREAK_CONNECT,
    CON_CONFIRM_NEW_NAME, CON_CONFIRM_NEW_PASSWORD, CON_GET_ANSI,
    CON_GET_NAME, CON_GET_NEW_PASSWORD, CON_GET_NEW_ROLE, CON_GET_OLD_PASSWORD,
    CON_PLAYING, CON_READ_MOTD, COMM_COMPACT, COMM_PROMPT, ECHO_OFF_STR,
    ECHO_ON_STR, LEVEL_HERO, MAX_INPUT_LENGTH, PLR_COLOUR, PLR_DENY,
    PLR_IS_NPC, PULSE_PER_SECOND, ROOM_VNUM_TEMPLE, TO_ROOM,
)
from .data import CharData, Descriptor, PcData
from .db import boot_db
from .handler import (
    char_to_room, get_curtime, get_trust, is_immortal, is_npc,
)
from .interp import interpret
from .log import bug, log_string, logf_string
from .save import load_char_obj, save_char_obj
from .update import update_handler
from .util import is_exact_name, is_name, str_cmp


def crypt_password(password: str, salt: str) -> str:
    """Hash a password. Uses SHA-256 for simplicity."""
    return hashlib.sha256((salt + password).encode()).hexdigest()


def check_parse_name(name: str) -> bool:
    """Parse a name for acceptability."""
    if is_exact_name(name, "all auto immortal self someone something the you"):
        return False
    if len(name) < 2:
        return False
    if len(name) > 12:
        return False

    all_il = True
    for ch in name:
        if not ch.isalpha():
            return False
        if ch.lower() not in ('i', 'l'):
            all_il = False

    if all_il:
        return False

    return True


def check_reconnect(d: Descriptor, name: str, f_conn: bool) -> bool:
    """Look for link-dead player to reconnect."""
    for ch in game.player_list:
        if (not f_conn or ch.desc is None) and not str_cmp(d.character.name, ch.name):
            if not f_conn:
                d.character.pcdata.pwd = ch.pcdata.pwd
            else:
                d.character = ch
                ch.desc = d
                ch.timer = 0
                send_to_char("Reconnecting.\r\n", ch)
                act("$n has reconnected.", ch, None, None, act_type=TO_ROOM)
                log_string(f"{ch.name}@{d.host} reconnected.")
                d.connected = CON_PLAYING
            return True

    return False


def check_playing(d: Descriptor, name: str) -> bool:
    """Check if already playing."""
    for dold in game.descriptor_list:
        if (dold is not d and dold.character is not None
                and dold.connected != CON_GET_NAME
                and dold.connected != CON_GET_OLD_PASSWORD
                and not str_cmp(name, dold.character.name)):
            write_to_buffer(d, "That character is already playing.\r\n"
                             "Disconnect that player? ")
            d.connected = CON_BREAK_CONNECT
            return True

    return False


def stop_idling(ch: CharData) -> None:
    """Move an idle character back from limbo."""
    from .handler import char_from_room

    if (ch is None or ch.desc is None
            or ch.desc.connected != CON_PLAYING
            or ch.was_in_room is None
            or ch.in_room is not game.get_room_index(2)):
        return

    ch.timer = 0
    char_from_room(ch)
    char_to_room(ch, ch.was_in_room)
    ch.was_in_room = None
    act("$n has returned from the void.", ch, None, None, act_type=TO_ROOM)


def write_to_buffer(d: Descriptor, txt: str) -> None:
    """Append text to a descriptor's output buffer."""
    if not d.fcommand and not d.outbuf:
        d.outbuf = "\r\n"

    d.outbuf += txt


def close_socket(d: Descriptor) -> None:
    """Close a connection."""
    ch = d.character

    if ch is not None:
        log_string(f"Closing link to {ch.name}.")
        if d.connected == CON_PLAYING:
            act("$n has lost $s link.", ch, None, None, act_type=TO_ROOM)
            ch.desc = None
        else:
            # Free the character that was in creation
            if ch in game.char_list:
                game.char_list.remove(ch)
            if ch in game.player_list:
                game.player_list.remove(ch)

    if d in game.descriptor_list:
        game.descriptor_list.remove(d)

    # Close the socket
    if d.writer is not None:
        try:
            d.writer.close()
        except Exception:
            pass


def nanny(d: Descriptor, argument: str) -> None:
    """Handle connections that haven't logged in yet (state machine)."""
    argument = argument.lstrip()
    ch = d.character

    if d.connected == CON_GET_ANSI:
        if argument and argument[0].lower() == 'n':
            d.ansi = False
        else:
            d.ansi = True

        if game.help_greeting:
            text = game.help_greeting
            if text and text[0] == '.':
                text = text[1:]
            write_to_buffer(d, text)

        d.connected = CON_GET_NAME
        return

    elif d.connected == CON_GET_NAME:
        if not argument:
            close_socket(d)
            return

        argument = argument[0].upper() + argument[1:]
        if not check_parse_name(argument):
            write_to_buffer(d, "Illegal name, try another.\r\nName: ")
            return

        f_old = load_char_obj(d, argument)
        ch = d.character

        if ch.act & PLR_DENY:
            log_string(f"Denying access to {argument}@{d.host}.")
            write_to_buffer(d, "You are denied access.\r\n")
            close_socket(d)
            return

        if check_reconnect(d, argument, False):
            f_old = True
        elif game.wizlock and get_trust(ch) < LEVEL_HERO:
            write_to_buffer(d, "The game is wizlocked.\r\n")
            close_socket(d)
            return

        if f_old:
            write_to_buffer(d, "Password: ")
            # In a real telnet, we'd send ECHO_OFF_STR here
            d.connected = CON_GET_OLD_PASSWORD
        else:
            if game.newlock:
                write_to_buffer(d, "The game is newlocked.\r\n")
                close_socket(d)
                return
            write_to_buffer(d, f"Did I get that right, {argument} (Y/N)? ")
            d.connected = CON_CONFIRM_NEW_NAME
        return

    elif d.connected == CON_GET_OLD_PASSWORD:
        write_to_buffer(d, "\r\n")
        if crypt_password(argument, ch.name) != ch.pcdata.pwd:
            # Also try plaintext match for legacy files
            if argument != ch.pcdata.pwd:
                write_to_buffer(d, "Wrong password.\r\n")
                close_socket(d)
                return

        if check_reconnect(d, ch.name, True):
            return
        if check_playing(d, ch.name):
            return

        log_string(f"{ch.name}@{d.host} has connected.")
        do_help(ch, "motd")
        d.connected = CON_READ_MOTD
        return

    elif d.connected == CON_BREAK_CONNECT:
        if not argument:
            return
        if argument[0].lower() == 'y':
            for dold in list(game.descriptor_list):
                if dold is d or dold.character is None:
                    continue
                if str_cmp(ch.name, dold.character.name):
                    continue
                close_socket(dold)
            write_to_buffer(d, "Disconnected.   Re-enter name: ")
            if d.character is not None:
                if d.character in game.char_list:
                    game.char_list.remove(d.character)
                if d.character in game.player_list:
                    game.player_list.remove(d.character)
                d.character = None
            d.connected = CON_GET_NAME
        elif argument[0].lower() == 'n':
            write_to_buffer(d, "Name: ")
            if d.character is not None:
                if d.character in game.char_list:
                    game.char_list.remove(d.character)
                if d.character in game.player_list:
                    game.player_list.remove(d.character)
                d.character = None
            d.connected = CON_GET_NAME
        else:
            write_to_buffer(d, "Please type Y or N? ")
        return

    elif d.connected == CON_CONFIRM_NEW_NAME:
        if not argument:
            return
        if argument[0].lower() == 'y':
            write_to_buffer(d, f"New character.\r\nGive me a password for {ch.name}: ")
            d.connected = CON_GET_NEW_PASSWORD
            if d.ansi:
                ch.act |= PLR_COLOUR
        elif argument[0].lower() == 'n':
            write_to_buffer(d, "Ok, what IS it, then? ")
            if d.character in game.char_list:
                game.char_list.remove(d.character)
            if d.character in game.player_list:
                game.player_list.remove(d.character)
            d.character = None
            d.connected = CON_GET_NAME
        else:
            write_to_buffer(d, "Please type Yes or No? ")
        return

    elif d.connected == CON_GET_NEW_PASSWORD:
        write_to_buffer(d, "\r\n")
        if len(argument) < 5:
            write_to_buffer(d, "Password must be at least five characters long.\r\nPassword: ")
            return

        pwd = crypt_password(argument, ch.name)
        if '~' in pwd:
            write_to_buffer(d, "New password not acceptable, try again.\r\nPassword: ")
            return

        ch.pcdata.pwd = pwd
        write_to_buffer(d, "Please retype password: ")
        d.connected = CON_CONFIRM_NEW_PASSWORD
        return

    elif d.connected == CON_CONFIRM_NEW_PASSWORD:
        write_to_buffer(d, "\r\n")
        if crypt_password(argument, ch.name) != ch.pcdata.pwd:
            write_to_buffer(d, "Passwords don't match.\r\nRetype password: ")
            d.connected = CON_GET_NEW_PASSWORD
            return

        write_to_buffer(d,
            "\r\nThe following roles are available:\r\n"
            "  [1] Wanderer  - A traveler and explorer\r\n"
            "  [2] Merchant  - A trader and craftsperson\r\n"
            "  [3] Scholar   - A seeker of knowledge\r\n"
            "  [4] Guardian  - A protector of the realm\r\n"
            "\r\nSelect a role: ")
        d.connected = CON_GET_NEW_ROLE
        return

    elif d.connected == CON_GET_NEW_ROLE:
        if not argument:
            return
        roles = {
            '1': " the Wanderer",
            '2': " the Merchant",
            '3': " the Scholar",
            '4': " the Guardian",
        }
        if argument[0] in roles:
            set_title(ch, roles[argument[0]])
        else:
            write_to_buffer(d, "That's not a valid role.\r\nSelect a role [1-4]: ")
            return

        log_string(f"{ch.name}@{d.host} new player.")
        write_to_buffer(d, "\r\n")
        do_help(ch, "motd")
        d.connected = CON_READ_MOTD
        return

    elif d.connected == CON_READ_MOTD:
        write_to_buffer(d, "\r\nWelcome to EmberMUD.\r\n")

        game.char_list.append(ch)
        game.player_list.append(ch)
        d.connected = CON_PLAYING

        if ch.pcdata and ch.pcdata.title:
            set_title(ch, ch.pcdata.title)

        if ch.level == 0:
            ch.level = 1
            char_to_room(ch, game.get_room_index(ROOM_VNUM_TEMPLE))
            send_to_char("\r\n", ch)
            save_char_obj(ch)
        elif ch.in_room is not None:
            char_to_room(ch, ch.in_room)
        else:
            char_to_room(ch, game.get_room_index(ROOM_VNUM_TEMPLE))

        act("$n has entered the game.", ch, None, None, act_type=TO_ROOM)
        do_look(ch, "auto")
        save_char_obj(ch)
        return

    else:
        bug(f"Nanny: bad d.connected {d.connected}.")
        close_socket(d)


def doparseprompt(ch: CharData) -> str:
    """Parse a player's custom prompt string."""
    if not ch.pcdata or not ch.pcdata.prompt:
        return "> "

    out = []
    prompt = ch.pcdata.prompt
    i = 0
    while i < len(prompt):
        if prompt[i] != '%':
            out.append(prompt[i])
            i += 1
            continue

        i += 1
        if i >= len(prompt):
            break

        code = prompt[i]
        i += 1

        if code == 'r':
            out.append("\r\n")
        elif code == 'T':
            out.append(get_curtime())
        elif code == '#':
            if is_immortal(ch) and ch.in_room is not None:
                out.append(str(ch.in_room.vnum))
        else:
            out.append('%')

    return "".join(out)


def process_output(d: Descriptor) -> str:
    """Prepare output for sending, including prompt."""
    output = ""

    if not game.merc_down:
        if d.connected == CON_PLAYING and d.character is not None:
            ch = d.character
            if not (ch.comm & COMM_COMPACT):
                output += "\r\n"
            if ch.comm & COMM_PROMPT:
                if not is_npc(ch) and ch.pcdata and ch.pcdata.prompt:
                    output += doparseprompt(ch)
                else:
                    output += "> "

    output += d.outbuf
    d.outbuf = ""
    return output


# ---------------------------------------------------------------------------
# Asyncio networking layer
# ---------------------------------------------------------------------------

async def handle_client(reader: asyncio.StreamReader,
                        writer: asyncio.StreamWriter) -> None:
    """Handle a new client connection."""
    addr = writer.get_extra_info('peername')
    host = f"{addr[0]}:{addr[1]}" if addr else "(unknown)"

    log_string(f"New connection from {host}")

    if check_ban(host):
        try:
            writer.write(b"Your site has been banned.\r\n")
            await writer.drain()
            writer.close()
        except Exception:
            pass
        return

    d = Descriptor(
        reader=reader,
        writer=writer,
        host=host,
        connected=CON_GET_ANSI,
        ansi=True,
    )

    game.descriptor_list.append(d)

    # Send initial messages
    write_to_buffer(d, CFG_CONNECT_MSG)
    write_to_buffer(d, CFG_ASK_ANSI)

    # Flush initial output
    await flush_output(d)


async def flush_output(d: Descriptor) -> bool:
    """Send buffered output to the client. Returns False if connection lost."""
    if not d.outbuf and not d.fcommand:
        return True

    output = process_output(d)
    if not output:
        return True

    colored = do_color(output, d.ansi)

    try:
        d.writer.write(colored.encode('latin-1', errors='replace'))
        await d.writer.drain()
        return True
    except (ConnectionError, OSError):
        return False


async def read_input(d: Descriptor) -> bool:
    """Try to read a line of input from a descriptor. Returns False on disconnect."""
    try:
        data = await asyncio.wait_for(d.reader.readline(), timeout=0.05)
    except asyncio.TimeoutError:
        return True
    except (ConnectionError, OSError):
        return False

    if not data:
        return False

    try:
        line = data.decode('latin-1', errors='replace')
    except Exception:
        return False

    # Strip telnet IAC sequences
    cleaned = []
    i = 0
    while i < len(line):
        if ord(line[i]) == 255 and i + 2 < len(line):  # IAC
            i += 3  # Skip IAC + command + option
            continue
        cleaned.append(line[i])
        i += 1
    line = "".join(cleaned)

    # Strip CR/LF and non-printable characters
    line = line.rstrip('\r\n')
    line = "".join(c for c in line if c.isprintable() or c == ' ')

    if not line:
        line = " "

    # Handle ! repeat
    if line == '!':
        line = d.inlast
    else:
        d.inlast = line

    d.incomm = line
    return True


async def game_loop(port: int) -> None:
    """Main game loop using asyncio."""
    # Bind to both IPv4 and IPv6 so clients connecting to either
    # 127.0.0.1 or ::1 (localhost) will work.
    server = await asyncio.start_server(
        handle_client, None, port, reuse_address=True,
    )

    logf_string("EmberMUD is ready to rock on port %d.", port)

    pulse_interval = 1.0 / PULSE_PER_SECOND

    try:
        while not game.merc_down:
            game.current_time = time.time()

            # Process input from all descriptors
            for d in list(game.descriptor_list):
                d.fcommand = False

                if d.character is not None:
                    d.character.timer = 0

                success = await read_input(d)
                if not success:
                    if d.character is not None and d.character.level > 1:
                        save_char_obj(d.character)
                    close_socket(d)
                    continue

                if d.character is not None and d.character.wait > 0:
                    d.character.wait -= 1
                    continue

                if d.incomm:
                    d.fcommand = True
                    stop_idling(d.character)

                    if d.connected == CON_PLAYING:
                        substitute_alias(d, d.incomm)
                    else:
                        nanny(d, d.incomm)

                    d.incomm = ""

            # Autonomous game motion
            update_handler()

            # Process output
            for d in list(game.descriptor_list):
                if d.fcommand or d.outbuf:
                    success = await flush_output(d)
                    if not success:
                        if d.character is not None and d.character.level > 1:
                            save_char_obj(d.character)
                        close_socket(d)

            # Synchronize to clock
            await asyncio.sleep(pulse_interval)
    finally:
        server.close()
        await server.wait_closed()


def main(port: int = 9000) -> None:
    """Entry point for the MUD server."""
    game.current_time = time.time()
    game.boot_time = get_curtime()

    boot_db()

    try:
        asyncio.run(game_loop(port))
    except KeyboardInterrupt:
        log_string("Received interrupt, shutting down.")
    except OSError as e:
        log_string(f"Error: {e}")
        if e.errno == 98:
            log_string(f"Port {port} is already in use. Kill the other process or pick a different port.")
        return

    log_string("Normal termination of game.")
