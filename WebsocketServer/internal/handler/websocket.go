package handler

import (
	"context"
	"encoding/json"
	"luchat/WebsocketServer/internal/global"
	"luchat/WebsocketServer/internal/model"
	"luchat/WebsocketServer/internal/service"
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"github.com/sirupsen/logrus"
)

var (
	// 将HTTP连接升级为WebSocket连接
	upgrader = websocket.Upgrader{
		CheckOrigin: func(r *http.Request) bool {
			return true // 允许所有跨域请求（生产环境需谨慎配置）
		},
		// 读写缓冲区大小
		ReadBufferSize:  1024,
		WriteBufferSize: 1024,
	}
	// 互斥锁：保护全局客户端集合的并发读写安全
	mu sync.Mutex
	// 心跳超时时间（60秒未收到心跳则断开）
	heartbeatTimeout = 60 * time.Second
)

// 处理WebSocket连接
func HandleWebsocket(ctx *gin.Context) {
	// 1. 将HTTP连接升级为WebSocket连接
	ws, err := upgrader.Upgrade(ctx.Writer, ctx.Request, nil)
	if err != nil {
		logrus.Printf("WebSocket升级失败:%v", err)
		ctx.AbortWithStatus(http.StatusInternalServerError)
		return
	}
	defer ws.Close() // 函数退出时关闭连接
	// 设置读超时（配合心跳检测）
	if err := ws.SetReadDeadline(time.Now().Add(heartbeatTimeout)); err != nil {
		logrus.Errorf("设置读超时失败: %v", err)
		return
	}
	// 2. 将新客户端注册到全局客户端集合
	mu.Lock() // 加锁，防止并发修改
	global.Clients[ws] = true
	mu.Unlock()

	// 心跳重置函数（每次收到消息时刷新超时时间）
	resetHeartbeat := func() {
		if err := ws.SetReadDeadline(time.Now().Add(heartbeatTimeout)); err != nil {
			logrus.Errorf("重置心跳超时失败: %v", err)
			// 失败时主动断开连接
			mu.Lock()
			delete(global.Clients, ws)
			mu.Unlock()
			ws.Close()
		}
	}

	// 3. 循环读取客户端发送的消息
	for {
		mt, msg, err := ws.ReadMessage()
		if err != nil {
			// 区分是超时错误还是其他错误
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				logrus.Errorf("WebSocket读取消息失败: %v", err)
			} else {
				logrus.Debugf("连接已关闭（可能是心跳超时）: %v", err)
			}
			// 移除客户端
			mu.Lock()
			delete(global.Clients, ws)
			mu.Unlock()
			break
		}
		// 判断是否为心跳包
		if mt == websocket.TextMessage {
			messageStr := string(msg)

			if messageStr == "ping" {
				// 回复pong
				if err := ws.WriteMessage(websocket.TextMessage, []byte("pong")); err != nil {
					logrus.Errorf("发送pong失败: %v", err)
					break
				}
				resetHeartbeat() // 重置超时
				continue         // 不广播心跳包
			}

			// 检查是否为AI对话请求
			if strings.HasPrefix(messageStr, "@ai ") || strings.HasPrefix(messageStr, "/ai ") {
				// 处理AI对话
				aiMessage := strings.TrimPrefix(messageStr, "@ai ")
				aiMessage = strings.TrimPrefix(aiMessage, "/ai ")

				go handleAIMessage(aiMessage)
				resetHeartbeat()
				continue
			}
		}
		// 重置心跳超时
		resetHeartbeat()
		// 将消息发送到全局广播通道，等待广播协程处理
		global.Broadcast <- global.StringMessage{
			MessageType: mt,  // 消息类型（与读取的一致）
			Message:     msg, // 消息内容
		}
	}
}

// 处理AI消息
func handleAIMessage(message string) {
	logrus.Infof("收到AI对话请求: %s", message)

	aiService := service.GetAIService()
	if aiService == nil {
		logrus.Error("AI服务未初始化")
		broadcastMessage("系统", "AI服务暂时不可用，请稍后重试")
		return
	}

	// 调用AI服务
	aiReq := &model.AIChatRequest{
		Message: message,
		Stream:  false, // WebSocket中暂时不支持流式输出
	}

	aiResp, err := aiService.Chat(context.Background(), aiReq)
	if err != nil {
		logrus.Errorf("AI对话失败: %v", err)
		broadcastMessage("豆包AI", "抱歉，我现在无法回答您的问题，请稍后重试。")
		return
	}

	// 广播AI回复
	broadcastMessage("豆包AI", aiResp.Message)
}

// 广播消息给所有客户端
func broadcastMessage(sender, content string) {
	message := global.Message{
		Email:    sender,
		Username: sender,
		Userid:   "ai",
		Message:  content,
	}

	messageBytes, err := json.Marshal(message)
	if err != nil {
		logrus.Errorf("序列化AI消息失败: %v", err)
		return
	}

	global.Broadcast <- global.StringMessage{
		MessageType: websocket.TextMessage,
		Message:     messageBytes,
	}
}

// 启动一个后台协程，负责将接收到的消息广播给所有在线客户端
func StartBroadCast() {
	// 循环从广播通道接收消息
	for msg := range global.Broadcast {
		// 加锁，防止并发修改客户端集合
		mu.Lock()
		// 遍历所有在线客户端，发送消息
		for client := range global.Clients {
			if err := client.WriteMessage(msg.MessageType, msg.Message); err != nil {
				logrus.Errorf("WebSocket发送消息失败: %v", err)
				// 发送失败时关闭连接
				client.Close()
				// 从集合中移除
				delete(global.Clients, client)
			}
		}
		mu.Unlock()
	}
}
