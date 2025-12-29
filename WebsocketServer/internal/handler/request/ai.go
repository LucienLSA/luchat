package request

type AIChatReq struct {
	Message        string `json:"message" binding:"required"`
	UserID         string `json:"user_id,omitempty"`
	ConversationID string `json:"conversation_id,omitempty"`
	Stream         bool   `json:"stream,omitempty"`
}
