package network

import (
	"bufio"
	"embermud/internal/game"
	"log"
	"net"
	"strings"
	"sync"
	"unicode"
)

// Desc implements game.Descriptor and manages a single client connection.
type Desc struct {
	Conn      net.Conn
	Host      string
	connected int
	ansi      bool
	character *game.CharData

	InputCh  chan string // Lines from client -> game loop
	OutputCh chan string // Text from game loop -> client
	Done     chan struct{}

	mu sync.Mutex // Protects outputBuf for batched writes

	outputBuf strings.Builder
	FCommand  bool

	InLast       string
	Repeat       int
	ShowStrHead  string
	ShowStrPoint int

	World *game.World
}

// NewDesc creates a new Desc from an accepted connection.
func NewDesc(conn net.Conn, world *game.World) *Desc {
	host := conn.RemoteAddr().String()
	// Strip port
	if h, _, err := net.SplitHostPort(host); err == nil {
		host = h
	}

	d := &Desc{
		Conn:      conn,
		Host:      host,
		connected: game.ConGetAnsi,
		InputCh:   make(chan string, 16),
		OutputCh:  make(chan string, 256),
		Done:      make(chan struct{}),
		World:     world,
	}

	return d
}

// StartIO launches the reader and writer goroutines.
func (d *Desc) StartIO() {
	go d.readerLoop()
	go d.writerLoop()
}

// readerLoop reads lines from the TCP connection and sends them to InputCh.
func (d *Desc) readerLoop() {
	defer func() {
		close(d.InputCh)
	}()

	scanner := bufio.NewScanner(d.Conn)
	scanner.Buffer(make([]byte, 4*game.MaxInputLength), 4*game.MaxInputLength)

	for {
		select {
		case <-d.Done:
			return
		default:
		}

		if !scanner.Scan() {
			return
		}

		raw := scanner.Bytes()
		// Strip telnet sequences
		clean := StripTelnet(raw)

		// Filter to printable characters, handle backspace
		var line []byte
		for _, b := range clean {
			if b == '\b' || b == 127 {
				if len(line) > 0 {
					line = line[:len(line)-1]
				}
			} else if b >= 32 && b < 127 {
				line = append(line, b)
			}
		}

		str := string(line)

		// Empty lines after stripping (telnet noise, bare CR/LF) are
		// sent as a single space only when we're in a paging state or
		// the nanny is waiting for input. Otherwise skip them to avoid
		// spamming prompts.
		if str == "" {
			str = " "
		}

		// Handle ! repeat
		if str == "!" {
			str = d.InLast
		} else if str != " " {
			d.InLast = str
		}

		select {
		case d.InputCh <- str:
		case <-d.Done:
			return
		}
	}
}

// writerLoop reads text from OutputCh and writes it to the TCP connection.
func (d *Desc) writerLoop() {
	for {
		select {
		case txt, ok := <-d.OutputCh:
			if !ok {
				return
			}
			colored := game.DoColor(txt, d.ansi)
			_, err := d.Conn.Write([]byte(colored))
			if err != nil {
				return
			}
		case <-d.Done:
			// Drain remaining output
			for {
				select {
				case txt, ok := <-d.OutputCh:
					if !ok {
						return
					}
					colored := game.DoColor(txt, d.ansi)
					d.Conn.Write([]byte(colored))
				default:
					return
				}
			}
		}
	}
}

// WriteToBuffer appends text to the output buffer (batched per tick).
func (d *Desc) WriteToBuffer(txt string) {
	d.mu.Lock()
	d.outputBuf.WriteString(txt)
	d.mu.Unlock()
}

// FlushOutput sends the buffered output to the OutputCh.
func (d *Desc) FlushOutput() {
	d.mu.Lock()
	txt := d.outputBuf.String()
	d.outputBuf.Reset()
	d.mu.Unlock()

	if txt == "" {
		return
	}

	select {
	case d.OutputCh <- txt:
	case <-d.Done:
	}
}

// Close shuts down the connection.
func (d *Desc) Close() {
	select {
	case <-d.Done:
		return // Already closed
	default:
	}
	close(d.Done)

	if d.character != nil && d.connected == game.ConPlaying {
		game.Act("$n has lost $s link.", d.character, nil, nil, game.ToRoom)
		d.character.Desc = nil
		log.Printf("Closing link to %s.", d.character.Name)
	}

	d.Conn.Close()
	d.World.RemoveDescriptor(d)
}

// Interface methods to satisfy game.Descriptor

func (d *Desc) GetConnected() int           { return d.connected }
func (d *Desc) SetConnected(state int)      { d.connected = state }
func (d *Desc) GetAnsi() bool               { return d.ansi }
func (d *Desc) SetAnsi(v bool)              { d.ansi = v }
func (d *Desc) GetCharacter() *game.CharData { return d.character }
func (d *Desc) SetCharacter(ch *game.CharData) { d.character = ch }
func (d *Desc) GetHost() string              { return d.Host }
func (d *Desc) GetShowStrPoint() int         { return d.ShowStrPoint }
func (d *Desc) SetShowStrPoint(v int)        { d.ShowStrPoint = v }
func (d *Desc) GetShowStrHead() string       { return d.ShowStrHead }
func (d *Desc) SetShowStrHead(v string)      { d.ShowStrHead = v }

// HasBufferedOutput returns true if there is pending output to send.
func (d *Desc) HasBufferedOutput() bool {
	d.mu.Lock()
	defer d.mu.Unlock()
	return d.outputBuf.Len() > 0
}

// ProcessOutput generates the prompt and flushes output.
// Should only be called when FCommand is true or there is buffered output.
func (d *Desc) ProcessOutput() {
	if d.World.MercDown {
		d.FlushOutput()
		return
	}

	if d.ShowStrHead != "" && d.ShowStrPoint < len(d.ShowStrHead) {
		d.WriteToBuffer("`W\n\r[Hit Return to continue]\n\r`0")
	} else if d.FCommand && d.connected == game.ConPlaying && d.character != nil {
		ch := d.character
		if !game.IsSet(ch.Comm, game.CommCompact) {
			d.WriteToBuffer("\n\r")
		}
		if game.IsSet(ch.Comm, game.CommPrompt) {
			if !ch.IsNPC() && ch.PCData != nil && ch.PCData.Prompt != "" {
				d.WriteToBuffer(game.DoParsePrompt(ch))
			} else {
				d.WriteToBuffer("> ")
			}
		}
	}

	d.FlushOutput()
}

// ShowString implements the pager.
func (d *Desc) ShowString(input string) {
	arg, _ := game.OneArgument(input)
	if arg != "" {
		d.ShowStrHead = ""
		d.ShowStrPoint = 0
		return
	}

	showLines := 0
	if d.character != nil {
		showLines = d.character.Lines
	}

	lines := 0
	text := d.ShowStrHead[d.ShowStrPoint:]
	toggle := 1

	var output strings.Builder
	for i := 0; i < len(text); i++ {
		ch := text[i]
		if (ch == '\n' || ch == '\r') && toggle < 0 {
			lines++
		}
		if ch == '\n' || ch == '\r' {
			toggle = -toggle
		}

		if showLines > 0 && lines >= showLines {
			d.ShowStrPoint += i
			d.WriteToBuffer(output.String())
			return
		}

		output.WriteByte(ch)
	}

	// Reached end of text
	d.WriteToBuffer(output.String())
	d.ShowStrHead = ""
	d.ShowStrPoint = 0
}

// PageToChar sets up the pager for a character.
func PageToChar(txt string, ch *game.CharData) {
	if ch.Desc == nil {
		return
	}
	desc := ch.Desc.(*Desc)
	desc.ShowStrHead = txt
	desc.ShowStrPoint = 0
	desc.ShowString("")
}

// SubstituteAlias checks for alias expansion.
func SubstituteAlias(d *Desc, input string, interpret func(ch *game.CharData, cmd string)) {
	ch := d.character
	if ch == nil || ch.IsNPC() || ch.PCData == nil {
		if ch != nil {
			interpret(ch, input)
		}
		return
	}

	arg, _ := game.OneArgument(input)

	for alias, sub := range ch.PCData.Aliases {
		if strings.EqualFold(arg, alias) {
			// Find remainder after the alias word
			point := input
			for len(point) > 0 && !isSpace(point[0]) {
				point = point[1:]
			}
			point = strings.TrimLeft(point, " \t")

			expanded := sub + " " + point
			interpret(ch, expanded)
			return
		}
	}

	interpret(ch, input)
}

func isSpace(b byte) bool {
	return unicode.IsSpace(rune(b))
}
