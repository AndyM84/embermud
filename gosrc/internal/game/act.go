package game

import (
	"fmt"
	"strings"
	"unicode"
)

var (
	heShe  = [3]string{"it", "he", "she"}
	himHer = [3]string{"it", "him", "her"}
	hisHer = [3]string{"its", "his", "her"}
)

func clampSex(sex int) int {
	if sex < 0 {
		return 0
	}
	if sex > 2 {
		return 2
	}
	return sex
}

// ActString formats a single act() message.
func ActString(format string, to, ch *CharData, arg1, arg2 interface{}) string {
	var buf strings.Builder
	vch, _ := arg2.(*CharData)

	for i := 0; i < len(format); i++ {
		if format[i] != '$' {
			buf.WriteByte(format[i])
			continue
		}
		i++
		if i >= len(format) {
			break
		}

		code := format[i]

		// Uppercase codes require arg2
		if arg2 == nil && code >= 'A' && code <= 'Z' {
			buf.WriteString(" <@@@> ")
			continue
		}

		var replacement string
		switch code {
		case 't':
			if s, ok := arg1.(string); ok {
				replacement = s
			}
		case 'T':
			if s, ok := arg2.(string); ok {
				replacement = s
			}
		case 'n':
			replacement = ch.Name
		case 'N':
			if vch != nil {
				replacement = vch.Name
			}
		case 'e':
			replacement = heShe[clampSex(ch.Sex)]
		case 'E':
			if vch != nil {
				replacement = heShe[clampSex(vch.Sex)]
			}
		case 'm':
			replacement = himHer[clampSex(ch.Sex)]
		case 'M':
			if vch != nil {
				replacement = himHer[clampSex(vch.Sex)]
			}
		case 's':
			replacement = hisHer[clampSex(ch.Sex)]
		case 'S':
			if vch != nil {
				replacement = hisHer[clampSex(vch.Sex)]
			}
		case 'd':
			if s, ok := arg2.(string); ok && s != "" {
				w, _ := OneArgument(s)
				replacement = w
			} else {
				replacement = "door"
			}
		default:
			replacement = fmt.Sprintf(" <@@@> ")
		}

		buf.WriteString(replacement)
	}

	// Append color reset and newline
	buf.WriteString("`w\n\r")

	result := buf.String()
	// Capitalize first character
	if len(result) > 0 {
		runes := []rune(result)
		runes[0] = unicode.ToUpper(runes[0])
		result = string(runes)
	}

	return result
}

// Act sends a formatted message to characters in a room.
func Act(format string, ch *CharData, arg1, arg2 interface{}, actType int) {
	ActNew(format, ch, arg1, arg2, actType, PosResting)
}

// ActNew sends a formatted message with minimum position check.
func ActNew(format string, ch *CharData, arg1, arg2 interface{}, actType, minPos int) {
	if format == "" || ch == nil {
		return
	}

	vch, _ := arg2.(*CharData)

	var targets []*CharData

	switch actType {
	case ToChar:
		targets = []*CharData{ch}
	case ToVict:
		if vch == nil || vch.InRoom == nil {
			return
		}
		targets = []*CharData{vch}
	default:
		if ch.InRoom == nil {
			return
		}
		targets = make([]*CharData, len(ch.InRoom.People))
		copy(targets, ch.InRoom.People)
	}

	for _, to := range targets {
		if to == nil || to.Desc == nil || to.Position < minPos {
			continue
		}
		if actType == ToChar && to != ch {
			continue
		}
		if actType == ToVict && (to != vch || to == ch) {
			continue
		}
		if actType == ToRoom && to == ch {
			continue
		}
		if actType == ToNotVict && (to == ch || to == vch) {
			continue
		}

		txt := ActString(format, to, ch, arg1, arg2)
		to.Desc.WriteToBuffer(txt)
	}
}
