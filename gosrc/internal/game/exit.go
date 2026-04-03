package game

// ExitData represents an exit from a room.
type ExitData struct {
	ToRoom      *RoomIndex // Resolved after FixExits
	ToVnum      int        // Used during area loading
	ExitInfo    int64
	Key         int
	Keyword     string
	Description string
}
