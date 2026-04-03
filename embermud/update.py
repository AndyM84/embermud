"""Game state updates - tick processing.

Ports update.c from the C version.
"""

from __future__ import annotations

from . import game
from .config import LEVEL_IMMORTAL, PULSE_TICK, ROOM_VNUM_LIMBO
from .handler import char_from_room, char_to_room, is_npc
from .save import save_char_obj


# Tick counter
_pulse_point = PULSE_TICK
_save_number = 0


def char_update() -> None:
    """Per-tick update for all characters."""
    global _save_number

    _save_number += 1
    if _save_number > 30:
        _save_number = 0

    ch_quit = None

    for ch in list(game.char_list):
        if is_npc(ch):
            continue

        # Increment played time
        ch.played += int(game.current_time - ch.logon)
        ch.logon = game.current_time

        # Increment idle timer
        ch.timer += 1

        # Idle timeout: move to limbo after 12 ticks
        if ch.timer >= 12 and ch.level < LEVEL_IMMORTAL:
            if ch.was_in_room is None and ch.in_room is not None:
                ch.was_in_room = ch.in_room
                from .comm import act, send_to_char
                act("$n disappears into the void.", ch, None, None, game=game)
                send_to_char("You disappear into the void.\r\n", ch)
                if ch.level > 1:
                    save_char_obj(ch)
                char_from_room(ch)
                char_to_room(ch, game.get_room_index(ROOM_VNUM_LIMBO))

        # Auto-quit after 30 ticks idle
        if ch.timer > 30 and ch.level < LEVEL_IMMORTAL:
            ch_quit = ch

    # Auto-save and auto-quit pass
    for ch in list(game.player_list):
        if ch.desc is not None and _save_number == 30:
            save_char_obj(ch)

        if ch is ch_quit:
            from .act_basic import do_quit
            do_quit(ch, "")


def update_handler() -> None:
    """Called once per pulse from the game loop."""
    global _pulse_point

    _pulse_point -= 1
    if _pulse_point <= 0:
        _pulse_point = PULSE_TICK
        char_update()
