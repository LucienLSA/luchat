package response

type AIChatResp struct {
	Message        string `json:"message"`
	ConversationID string `json:"conversation_id"`
	FinishReason   string `json:"finish_reason,omitempty"`
}
