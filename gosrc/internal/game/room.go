package game

// RoomIndex represents a room in the world.
type RoomIndex struct {
	Vnum        int
	Name        string
	Description string
	RoomFlags   int64
	Light       int
	SectorType  int
	Exits       [MaxDir]*ExitData
	People      []*CharData
}

// AddPerson adds a character to the room's people list.
func (r *RoomIndex) AddPerson(ch *CharData) {
	r.People = append(r.People, ch)
}

// RemovePerson removes a character from the room's people list.
func (r *RoomIndex) RemovePerson(ch *CharData) {
	for i, p := range r.People {
		if p == ch {
			r.People = append(r.People[:i], r.People[i+1:]...)
			return
		}
	}
}
