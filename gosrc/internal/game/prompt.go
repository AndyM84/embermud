package game

import (
	"fmt"
	"strings"
	"time"
)

// DoParsePrompt parses a prompt string with token substitution.
func DoParsePrompt(ch *CharData) string {
	if ch.PCData == nil || ch.PCData.Prompt == "" {
		return "> "
	}

	prompt := ch.PCData.Prompt
	var b strings.Builder

	for i := 0; i < len(prompt); i++ {
		if prompt[i] != '%' {
			b.WriteByte(prompt[i])
			continue
		}

		i++
		if i >= len(prompt) {
			break
		}

		switch prompt[i] {
		case 'r':
			b.WriteString("\n\r")
		case 'T':
			b.WriteString(time.Now().Format("Mon Jan 2 15:04:05 2006"))
		case '#':
			if ch.IsImmortal() && ch.InRoom != nil {
				b.WriteString(fmt.Sprintf("%d", ch.InRoom.Vnum))
			}
		case '%':
			b.WriteByte('%')
		default:
			b.WriteByte('%')
		}
	}

	return b.String()
}
