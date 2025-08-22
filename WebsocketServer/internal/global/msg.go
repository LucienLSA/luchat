package global

// 消息结构
type Message struct {
	Email    string `json:"email"`
	Username string `json:"username"`
	Userid   string `json:"userid"`
	Message  string `json:"message"`
	Filelink string `json:"filelink"`
	Image    string `json:"image"`
}

type StringMessage struct {
	MessageType int
	Message     []byte
}
