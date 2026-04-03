package game

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

// BanData represents an IP ban.
type BanData struct {
	Name     string
	Level    int
	BanFlags int
}

// LoadBans loads bans from the ban file.
func LoadBans(filename string) []*BanData {
	f, err := os.Open(filename)
	if err != nil {
		return nil
	}
	defer f.Close()

	var bans []*BanData
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.TrimSpace(line) == "" {
			continue
		}
		ban := &BanData{}
		_, err := fmt.Sscanf(line, "%s %d %d", &ban.Name, &ban.Level, &ban.BanFlags)
		if err != nil {
			continue
		}
		bans = append(bans, ban)
	}
	return bans
}

// SaveBans writes permanent bans to disk.
func SaveBans(filename string, bans []*BanData) {
	var permanent []*BanData
	for _, b := range bans {
		if b.BanFlags&BanPermanent != 0 {
			permanent = append(permanent, b)
		}
	}

	if len(permanent) == 0 {
		os.Remove(filename)
		return
	}

	f, err := os.Create(filename)
	if err != nil {
		return
	}
	defer f.Close()

	for _, b := range permanent {
		fmt.Fprintf(f, "%-20s %-2d %d\n", b.Name, b.Level, b.BanFlags)
	}
}

// CheckBan checks if a host is banned. Returns true if banned.
func CheckBan(host string, bans []*BanData) bool {
	h := strings.ToLower(host)

	for _, ban := range bans {
		if ban.BanFlags&BanAll == 0 {
			continue
		}

		name := strings.ToLower(ban.Name)

		if ban.BanFlags&BanPrefix != 0 && ban.BanFlags&BanSuffix != 0 {
			if strings.Contains(h, name) {
				return true
			}
		} else if ban.BanFlags&BanPrefix != 0 {
			if strings.HasSuffix(h, name) {
				return true
			}
		} else if ban.BanFlags&BanSuffix != 0 {
			if strings.HasPrefix(h, name) {
				return true
			}
		}
	}
	return false
}
