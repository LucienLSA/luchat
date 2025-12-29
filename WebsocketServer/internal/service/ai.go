package service

import (
	"bufio"
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"luchat/WebsocketServer/config"
	"luchat/WebsocketServer/internal/model"
	"net/http"
	"strings"
	"time"

	"github.com/google/uuid"
	"github.com/sirupsen/logrus"
)

type AIService struct {
	config config.AIConfig
	client *http.Client
}

var aiService *AIService

// InitAIService 初始化AI服务
func InitAIService(cfg config.AIConfig) {
	aiService = &AIService{
		config: cfg,
		client: &http.Client{
			Timeout: time.Duration(cfg.Timeout) * time.Second,
		},
	}
}

// GetAIService 获取AI服务实例
func GetAIService() *AIService {
	return aiService
}

// Chat 发送对话请求到豆包AI
func (s *AIService) Chat(ctx context.Context, req *model.AIChatRequest) (*model.AIChatResponse, error) {
	if s == nil {
		return nil, fmt.Errorf("AI service not initialized")
	}

	// 构建对话消息
	messages := []model.Message{
		{
			Role:    "system",
			Content: "你是豆包，是由字节跳动开发的AI助手。请友好地回答用户的问题。",
		},
		{
			Role:    "user",
			Content: req.Message,
		},
	}

	// 构建API请求
	doubaoReq := model.DoubaoChatRequest{
		Model:       s.config.Model,
		Messages:    messages,
		Stream:      req.Stream,
		Temperature: s.config.Temperature,
		MaxTokens:   s.config.MaxTokens,
	}

	// 发送请求
	resp, err := s.sendRequest(ctx, doubaoReq)
	if err != nil {
		logrus.Errorf("AI chat request failed: %v", err)
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		logrus.Errorf("AI API error: status=%d, body=%s", resp.StatusCode, string(body))
		return nil, fmt.Errorf("AI API error: %d", resp.StatusCode)
	}

	// 解析响应
	if req.Stream {
		// 流式响应处理
		return s.handleStreamResponse(resp.Body)
	} else {
		// 非流式响应处理
		return s.handleNormalResponse(resp.Body)
	}
}

// sendRequest 发送HTTP请求到豆包AI API
func (s *AIService) sendRequest(ctx context.Context, req model.DoubaoChatRequest) (*http.Response, error) {
	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("marshal request failed: %v", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", s.config.BaseURL+"/chat/completions", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("create request failed: %v", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")
	httpReq.Header.Set("Authorization", "Bearer "+s.config.APIKey)

	return s.client.Do(httpReq)
}

// handleNormalResponse 处理非流式响应
func (s *AIService) handleNormalResponse(body io.Reader) (*model.AIChatResponse, error) {
	var doubaoResp model.DoubaoChatResponse
	if err := json.NewDecoder(body).Decode(&doubaoResp); err != nil {
		return nil, fmt.Errorf("decode response failed: %v", err)
	}

	if len(doubaoResp.Choices) == 0 {
		return nil, fmt.Errorf("no response from AI")
	}

	conversationID := uuid.New().String()

	return &model.AIChatResponse{
		Message:        doubaoResp.Choices[0].Message.Content,
		ConversationID: conversationID,
		FinishReason:   doubaoResp.Choices[0].FinishReason,
	}, nil
}

// handleStreamResponse 处理流式响应
func (s *AIService) handleStreamResponse(body io.Reader) (*model.AIChatResponse, error) {
	reader := bufio.NewReader(body)
	var fullContent strings.Builder
	conversationID := uuid.New().String()

	for {
		line, err := reader.ReadString('\n')
		if err != nil {
			if err == io.EOF {
				break
			}
			return nil, fmt.Errorf("read stream failed: %v", err)
		}

		line = strings.TrimSpace(line)
		if line == "" || !strings.HasPrefix(line, "data: ") {
			continue
		}

		data := strings.TrimPrefix(line, "data: ")
		if data == "[DONE]" {
			break
		}

		var streamResp model.DoubaoStreamResponse
		if err := json.Unmarshal([]byte(data), &streamResp); err != nil {
			logrus.Warnf("Failed to unmarshal stream data: %v", err)
			continue
		}

		if len(streamResp.Choices) > 0 {
			delta := streamResp.Choices[0].Delta
			if delta.Content != "" {
				fullContent.WriteString(delta.Content)
			}
		}
	}

	return &model.AIChatResponse{
		Message:        fullContent.String(),
		ConversationID: conversationID,
		FinishReason:   "stop",
	}, nil
}

// HealthCheck 检查AI服务健康状态
func (s *AIService) HealthCheck(ctx context.Context) error {
	testReq := model.AIChatRequest{
		Message: "Hello",
		Stream:  false,
	}

	_, err := s.Chat(ctx, &testReq)
	return err
}
