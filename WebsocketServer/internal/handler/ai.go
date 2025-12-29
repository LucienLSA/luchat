package handler

import (
	"luchat/WebsocketServer/internal/handler/request"
	"luchat/WebsocketServer/internal/handler/response"
	"luchat/WebsocketServer/internal/model"
	"luchat/WebsocketServer/internal/service"

	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
)

// AIChat 处理AI对话请求
func AIChat(c *gin.Context) {
	var req request.AIChatReq
	if err := c.ShouldBindJSON(&req); err != nil {
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	aiService := service.GetAIService()
	if aiService == nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	// 构建AI请求
	aiReq := &model.AIChatRequest{
		Message:     req.Message,
		UserID:      req.UserID,
		ConversationID: req.ConversationID,
		Stream:      req.Stream,
	}

	// 调用AI服务
	aiResp, err := aiService.Chat(c.Request.Context(), aiReq)
	if err != nil {
		logrus.Errorf("AI chat failed: %v", err)
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	// 返回响应
	resp := &response.AIChatResp{
		Message:        aiResp.Message,
		ConversationID: aiResp.ConversationID,
		FinishReason:   aiResp.FinishReason,
	}

	response.ResponseSuccessData(c, resp)
}

// AIHealthCheck AI服务健康检查
func AIHealthCheck(c *gin.Context) {
	aiService := service.GetAIService()
	if aiService == nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	if err := aiService.HealthCheck(c.Request.Context()); err != nil {
		logrus.Errorf("AI health check failed: %v", err)
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	response.ResponseSuccessData(c, gin.H{"status": "ok", "message": "AI service is healthy"})
}
