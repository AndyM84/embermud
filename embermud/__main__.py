"""Entry point for running EmberMUD as a Python module.

Usage: python -m embermud [port]
"""

import sys

from .comm import main


if __name__ == "__main__":
    port = 9000
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print(f"Usage: {sys.argv[0]} [port #]", file=sys.stderr)
            sys.exit(1)
        if port <= 1024:
            print("Port number must be above 1024.", file=sys.stderr)
            sys.exit(1)

    main(port)
