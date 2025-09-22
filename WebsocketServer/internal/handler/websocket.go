package handler

import (
	"luchat/WebsocketServer/internal/global"
	"net/http"
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
			if string(msg) == "ping" {
				// 回复pong
				if err := ws.WriteMessage(websocket.TextMessage, []byte("pong")); err != nil {
					logrus.Errorf("发送pong失败: %v", err)
					break
				}
				resetHeartbeat() // 重置超时
				continue         // 不广播心跳包
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
