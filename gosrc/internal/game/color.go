package game

import "strings"

// colorMap maps backtick color codes to ANSI escape sequences.
var colorMap = map[byte]string{
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

// DoColor processes backtick color codes in text.
// If color is true, converts to ANSI escapes. If false, strips them.
func DoColor(plaintext string, color bool) string {
	if plaintext == "" {
		return ""
	}

	var b strings.Builder
	b.Grow(len(plaintext) * 2)

	for i := 0; i < len(plaintext); i++ {
		if plaintext[i] != '`' {
			b.WriteByte(plaintext[i])
			continue
		}

		i++
		if i >= len(plaintext) {
			break
		}

		if !color {
			// Strip color codes; pass through literal backtick
			if plaintext[i] == '`' {
				b.WriteByte('`')
			}
			continue
		}

		ch := plaintext[i]
		if ch == '`' {
			b.WriteByte('`')
		} else if ansi, ok := colorMap[ch]; ok {
			b.WriteString(ansi)
		} else {
			// Unknown code — pass through
			b.WriteByte('`')
			b.WriteByte(ch)
		}
	}

	if color {
		b.WriteString("\033[0m")
	}

	return b.String()
}
