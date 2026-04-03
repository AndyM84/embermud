package persist

import (
	"bufio"
	"log"
	"unicode"
)

// FreadLetter reads a non-whitespace character.
func FreadLetter(r *bufio.Reader) byte {
	for {
		b, err := r.ReadByte()
		if err != nil {
			return 0
		}
		if !unicode.IsSpace(rune(b)) {
			return b
		}
	}
}

// FreadNumber reads an integer, supporting +/- signs and | (OR) chains.
func FreadNumber(r *bufio.Reader) int {
	// Skip whitespace
	var c byte
	for {
		b, err := r.ReadByte()
		if err != nil {
			return 0
		}
		if !unicode.IsSpace(rune(b)) {
			c = b
			break
		}
	}

	sign := 1
	if c == '+' {
		b, err := r.ReadByte()
		if err != nil {
			return 0
		}
		c = b
	} else if c == '-' {
		sign = -1
		b, err := r.ReadByte()
		if err != nil {
			return 0
		}
		c = b
	}

	if !unicode.IsDigit(rune(c)) {
		log.Printf("fread_number: bad format, got '%c'", c)
		return 0
	}

	number := 0
	for unicode.IsDigit(rune(c)) {
		number = number*10 + int(c-'0')
		b, err := r.ReadByte()
		if err != nil {
			break
		}
		c = b
	}

	number *= sign

	if c == '|' {
		number += FreadNumber(r)
	} else if c != ' ' {
		r.UnreadByte()
	}

	return number
}

// FreadFlag reads a flag value (supports alpha flag encoding from ROM).
func FreadFlag(r *bufio.Reader) int64 {
	// Skip whitespace
	var c byte
	for {
		b, err := r.ReadByte()
		if err != nil {
			return 0
		}
		if !unicode.IsSpace(rune(b)) {
			c = b
			break
		}
	}

	var number int64

	if !unicode.IsDigit(rune(c)) && c != '-' {
		// Alpha flag encoding
		for (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') {
			var bitsum int64
			if c >= 'A' && c <= 'Z' {
				bitsum = 1
				for i := c; i > 'A'; i-- {
					bitsum *= 2
				}
			} else if c >= 'a' && c <= 'z' {
				bitsum = 67108864 // 2^26
				for i := c; i > 'a'; i-- {
					bitsum *= 2
				}
			}
			number += bitsum
			b, err := r.ReadByte()
			if err != nil {
				return number
			}
			c = b
		}
	}

	if c == '-' {
		n := FreadNumber(r)
		return -int64(n)
	}

	for unicode.IsDigit(rune(c)) {
		number = number*10 + int64(c-'0')
		b, err := r.ReadByte()
		if err != nil {
			return number
		}
		c = b
	}

	if c == '|' {
		number += FreadFlag(r)
	} else if c != ' ' {
		r.UnreadByte()
	}

	return number
}

// FreadString reads a tilde-terminated string.
func FreadString(r *bufio.Reader) string {
	// Skip leading whitespace
	for {
		b, err := r.ReadByte()
		if err != nil {
			return ""
		}
		if !unicode.IsSpace(rune(b)) {
			r.UnreadByte()
			break
		}
	}

	var result []byte
	for {
		b, err := r.ReadByte()
		if err != nil {
			break
		}
		if b == '~' {
			break
		}
		result = append(result, b)
	}
	return string(result)
}

// FreadWord reads a whitespace-delimited word. Supports quoted strings.
func FreadWord(r *bufio.Reader) string {
	// Skip whitespace
	var cEnd byte
	for {
		b, err := r.ReadByte()
		if err != nil {
			return ""
		}
		if !unicode.IsSpace(rune(b)) {
			if b == '\'' || b == '"' {
				cEnd = b
			} else {
				cEnd = ' '
				r.UnreadByte()
			}
			break
		}
	}

	var word []byte
	for {
		b, err := r.ReadByte()
		if err != nil {
			break
		}
		if cEnd == ' ' {
			if unicode.IsSpace(rune(b)) {
				r.UnreadByte()
				break
			}
		} else {
			if b == cEnd {
				break
			}
		}
		word = append(word, b)
	}

	return string(word)
}

// FreadToEol reads to end of line.
func FreadToEol(r *bufio.Reader) {
	for {
		b, err := r.ReadByte()
		if err != nil || b == '\n' || b == '\r' {
			break
		}
	}
	// Skip additional \n or \r
	for {
		b, err := r.ReadByte()
		if err != nil {
			return
		}
		if b != '\n' && b != '\r' {
			r.UnreadByte()
			return
		}
	}
}
