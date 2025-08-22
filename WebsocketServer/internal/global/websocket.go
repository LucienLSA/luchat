package global

import (
	"github.com/gorilla/websocket"
)

// 客户端连接管理
// 存储所有连接的WebSocket客户端
// map键为WebSocket连接指针，快速查找和管理活跃连接
var Clients = make(map[*websocket.Conn]bool)

// goroutine之间传递广播消息
var Broadcast = make(chan StringMessage)
var OnlineUsers []OnlineUser
