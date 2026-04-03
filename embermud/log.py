"""Logging utilities for EmberMUD."""

import sys
import time


def log_string(msg: str) -> None:
    """Write a timestamped message to stderr."""
    strtime = time.strftime("%c", time.localtime())
    print(f"{strtime} :: {msg}", file=sys.stderr, flush=True)


def bug(msg: str, param: int = 0) -> None:
    """Report a bug."""
    log_string(f"[*****] BUG: {msg} {param}")


def logf_string(fmt: str, *args) -> None:
    """Log a formatted string."""
    log_string(fmt % args if args else fmt)
