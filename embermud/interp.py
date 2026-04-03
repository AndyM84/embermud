"""Command interpreter - parses and dispatches player commands.

Ports interp.c from the C version.
"""

from __future__ import annotations

from .config import (
    LOG_ALWAYS, LOG_NEVER, LOG_NORMAL, POS_DEAD, POS_RESTING, POS_STANDING,
)
from .data import CharData
from .log import log_string
from .util import str_prefix

# Import command functions
from .act_basic import (
    check_social, do_down, do_east, do_exits, do_help, do_look, do_north,
    do_quit, do_save, do_say, do_south, do_tell, do_up, do_west, do_who,
    send_to_char,
)


# Command table: (name, function, min_position, min_level, log_type)
cmd_table = [
    # Movement commands (priority at top for speed)
    ("north",  do_north,  POS_STANDING, 0, LOG_NEVER),
    ("east",   do_east,   POS_STANDING, 0, LOG_NEVER),
    ("south",  do_south,  POS_STANDING, 0, LOG_NEVER),
    ("west",   do_west,   POS_STANDING, 0, LOG_NEVER),
    ("up",     do_up,     POS_STANDING, 0, LOG_NEVER),
    ("down",   do_down,   POS_STANDING, 0, LOG_NEVER),

    # Common commands
    ("look",   do_look,   POS_RESTING,  0, LOG_NORMAL),
    ("exits",  do_exits,  POS_RESTING,  0, LOG_NORMAL),
    ("say",    do_say,    POS_RESTING,  0, LOG_NORMAL),
    ("'",      do_say,    POS_RESTING,  0, LOG_NORMAL),
    ("tell",   do_tell,   POS_RESTING,  0, LOG_NORMAL),
    ("who",    do_who,    POS_DEAD,     0, LOG_NORMAL),
    ("save",   do_save,   POS_DEAD,     0, LOG_NORMAL),
    ("quit",   do_quit,   POS_DEAD,     0, LOG_NORMAL),
    ("help",   do_help,   POS_DEAD,     0, LOG_NORMAL),
]


def interpret(ch: CharData, argument: str) -> None:
    """The main command interpreter."""
    argument = argument.lstrip()
    if not argument:
        return

    logline = argument

    # Grab the command word
    if not argument[0].isalpha() and not argument[0].isdigit():
        command = argument[0]
        argument = argument[1:].lstrip()
    else:
        # Extract first word
        parts = argument.split(None, 1)
        command = parts[0].lower()
        argument = parts[1] if len(parts) > 1 else ""

    # Look up command in the table
    found = False
    for cmd_name, cmd_fun, cmd_pos, cmd_level, cmd_log in cmd_table:
        if command[0] == cmd_name[0] and not str_prefix(command, cmd_name):
            # Check position
            if ch.position < cmd_pos:
                send_to_char("You can't do that right now.\r\n", ch)
                return

            # Check level
            if ch.level < cmd_level:
                send_to_char("Huh?\r\n", ch)
                return

            # Log if needed
            if cmd_log == LOG_ALWAYS:
                log_string(f"Log {ch.name}: {logline}")

            # Dispatch
            cmd_fun(ch, argument)
            found = True
            break

    if not found:
        if not check_social(ch, command, argument):
            send_to_char("Huh?\r\n", ch)
