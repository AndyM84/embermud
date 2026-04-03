"""ANSI color code processing.

Converts backtick color codes (e.g. `R for bright red) to ANSI escape
sequences, or strips them if color is disabled.
"""

# Map of backtick color codes to ANSI escape sequences
COLOR_TABLE = {
    '0': "\033[0m",
    'k': "\033[0;30m",
    'K': "\033[1;30m",
    'r': "\033[0;31m",
    'R': "\033[1;31m",
    'g': "\033[0;32m",
    'G': "\033[1;32m",
    'y': "\033[0;33m",
    'Y': "\033[1;33m",
    'b': "\033[0;34m",
    'B': "\033[1;34m",
    'm': "\033[0;35m",
    'M': "\033[1;35m",
    'c': "\033[0;36m",
    'C': "\033[1;36m",
    'w': "\033[0;37m",
    'W': "\033[1;37m",
}


def do_color(text: str, use_ansi: bool) -> str:
    """Process backtick color codes in text.

    If use_ansi is True, converts codes to ANSI escape sequences.
    If False, strips the codes entirely.
    """
    if not text:
        return ""

    out = []
    i = 0
    while i < len(text):
        if text[i] != '`':
            out.append(text[i])
            i += 1
            continue

        i += 1
        if i >= len(text):
            break

        code = text[i]
        i += 1

        if not use_ansi:
            if code == '`':
                out.append('`')
            # Otherwise strip the code
        else:
            if code == '`':
                out.append('`')
            elif code in COLOR_TABLE:
                out.append(COLOR_TABLE[code])
            else:
                out.append('`')
                out.append(code)

    if use_ansi:
        out.append("\033[0m")

    return "".join(out)
