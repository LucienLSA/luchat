package model

// 豆包AI API请求结构
type DoubaoChatRequest struct {
	Model       string    `json:"model"`
	Messages    []Message `json:"messages"`
	Stream      bool      `json:"stream,omitempty"`
	Temperature float32   `json:"temperature,omitempty"`
	MaxTokens   int       `json:"max_tokens,omitempty"`
}

type Message struct {
	Role    string `json:"role"`    // system, user, assistant
	Content string `json:"content"`
}

// 豆包AI API响应结构
type DoubaoChatResponse struct {
	ID      string   `json:"id"`
	Object  string   `json:"object"`
	Created int64    `json:"created"`
	Model   string   `json:"model"`
	Choices []Choice `json:"choices"`
	Usage   Usage    `json:"usage"`
}

type Choice struct {
	Index        int     `json:"index"`
	Message      Message `json:"message"`
	FinishReason string  `json:"finish_reason"`
}

type Usage struct {
	PromptTokens     int `json:"prompt_tokens"`
	CompletionTokens int `json:"completion_tokens"`
	TotalTokens      int `json:"total_tokens"`
}

// 流式响应结构
type DoubaoStreamResponse struct {
	ID      string      `json:"id"`
	Object  string      `json:"object"`
	Created int64       `json:"created"`
	Model   string      `json:"model"`
	Choices []StreamChoice `json:"choices"`
}

type StreamChoice struct {
	Index int           `json:"index"`
	Delta StreamMessage `json:"delta"`
	FinishReason *string `json:"finish_reason,omitempty"`
}

type StreamMessage struct {
	Role    string `json:"role,omitempty"`
	Content string `json:"content,omitempty"`
}

// 对话历史记录
type Conversation struct {
	ID          string    `json:"id"`
	UserID      string    `json:"user_id"`
	Messages    []Message `json:"messages"`
	CreatedAt   int64     `json:"created_at"`
	UpdatedAt   int64     `json:"updated_at"`
	MaxMessages int       `json:"max_messages,omitempty"` // 最大消息数量，默认20
}

// AI对话请求
type AIChatRequest struct {
	Message     string `json:"message" binding:"required"`
	UserID      string `json:"user_id,omitempty"`
	ConversationID string `json:"conversation_id,omitempty"`
	Stream      bool   `json:"stream,omitempty"`
}

// AI对话响应
type AIChatResponse struct {
	Message        string `json:"message"`
	ConversationID string `json:"conversation_id"`
	FinishReason   string `json:"finish_reason,omitempty"`
}
