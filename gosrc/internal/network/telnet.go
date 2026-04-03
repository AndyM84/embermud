package network

// Telnet protocol constants.
const (
	IAC         = 255
	WILL        = 251
	WONT        = 252
	DO          = 253
	DONT        = 254
	GA          = 249
	TELOPT_ECHO = 1
)

// Telnet control sequences.
var (
	EchoOffStr = string([]byte{IAC, WILL, TELOPT_ECHO})
	EchoOnStr  = string([]byte{IAC, WONT, TELOPT_ECHO})
	GoAheadStr = string([]byte{IAC, GA})
)

// StripTelnet removes IAC sequences from raw input.
func StripTelnet(data []byte) []byte {
	var out []byte
	i := 0
	for i < len(data) {
		if data[i] == IAC && i+1 < len(data) {
			cmd := data[i+1]
			switch cmd {
			case WILL, WONT, DO, DONT:
				// Skip 3-byte sequence: IAC CMD OPTION
				if i+2 < len(data) {
					i += 3
				} else {
					i += 2
				}
			case IAC:
				// Escaped IAC
				out = append(out, IAC)
				i += 2
			default:
				// 2-byte command
				i += 2
			}
		} else {
			out = append(out, data[i])
			i++
		}
	}
	return out
}
