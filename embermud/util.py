"""String and number utility functions for EmberMUD.

Ports the utility functions from db.c and interp.c.
"""

import random


def str_cmp(a: str, b: str) -> bool:
    """Case-insensitive string comparison. Returns True if DIFFERENT."""
    if a is None or b is None:
        return True
    return a.lower() != b.lower()


def str_prefix(astr: str, bstr: str) -> bool:
    """Check if astr is NOT a prefix of bstr (case-insensitive).
    Returns True if NOT a prefix (matching C convention).
    """
    if not astr or not bstr:
        return True
    return not bstr.lower().startswith(astr.lower())


def str_infix(astr: str, bstr: str) -> bool:
    """Check if astr is NOT found anywhere in bstr (case-insensitive).
    Returns True if NOT found.
    """
    if not astr:
        return False
    return astr.lower() not in bstr.lower()


def str_suffix(astr: str, bstr: str) -> bool:
    """Check if astr is NOT a suffix of bstr (case-insensitive).
    Returns True if NOT a suffix.
    """
    if not astr or not bstr:
        return True
    return not bstr.lower().endswith(astr.lower())


def capitalize(s: str) -> str:
    """Return string with first character uppercased, rest lowercased."""
    if not s:
        return s
    return s[0].upper() + s[1:].lower()


def is_number(arg: str) -> bool:
    """Check if a string is a valid integer."""
    if not arg:
        return False
    s = arg.lstrip("+-")
    return s.isdigit() if s else False


def one_argument(argument: str) -> tuple[str, str]:
    """Pick off one argument from a string.

    Returns (rest_of_string, extracted_word_lowercased).
    Handles single-quote grouping.
    """
    argument = argument.lstrip()

    if not argument:
        return ("", "")

    if argument[0] == "'":
        end_char = "'"
        argument = argument[1:]
    else:
        end_char = " "

    arg_first = []
    rest = argument
    for i, ch in enumerate(argument):
        if ch == end_char:
            rest = argument[i + 1:]
            break
        arg_first.append(ch.lower())
    else:
        rest = ""

    return (rest.lstrip(), "".join(arg_first))


def number_argument(argument: str) -> tuple[int, str]:
    """Parse 'N.keyword' syntax.

    Returns (count, keyword). If no dot, returns (1, argument).
    """
    dot = argument.find('.')
    if dot >= 0:
        try:
            number = int(argument[:dot])
        except ValueError:
            number = 1
        return (number, argument[dot + 1:])
    return (1, argument)


def smash_tilde(s: str) -> str:
    """Replace tildes with dashes."""
    return s.replace('~', '-')


def is_name(name: str, namelist: str) -> bool:
    """Check if name matches any word in namelist (prefix match).

    Each word in name must prefix-match some word in namelist.
    """
    if not name or not namelist:
        return False

    # Extract first word from name
    rest, part = one_argument(name)
    if not part:
        return True

    # Check if this word matches any word in namelist
    nlist = namelist
    while nlist:
        nlist, nword = one_argument(nlist)
        if not nword:
            return False
        if not str_prefix(part, nword):
            return True

    return False


def is_exact_name(name: str, namelist: str) -> bool:
    """Check if name exactly matches any word in namelist."""
    if not name or not namelist:
        return False

    rest, part = one_argument(name)
    if not part:
        return True

    nlist = namelist
    while nlist:
        nlist, nword = one_argument(nlist)
        if not nword:
            return False
        if not str_cmp(part, nword):
            return True

    return False


def number_range(low: int, high: int) -> int:
    """Generate a random number in [low, high]."""
    if low == high:
        return low
    if low > high:
        low, high = high, low
    return random.randint(low, high)


def number_percent() -> int:
    """Generate a random percentile (1-100)."""
    return random.randint(1, 100)


def number_door() -> int:
    """Generate a random door (0-5)."""
    return random.randint(0, 5)


def dice(number: int, size: int) -> int:
    """Roll some dice."""
    if size <= 0:
        return 0
    if size == 1:
        return number
    return sum(random.randint(1, size) for _ in range(number))
