# WebSocket 知识点总结

## 📚 目录
1. [WebSocket 基础概念](#websocket-基础概念)
2. [服务端实现](#服务端实现)
3. [客户端实现](#客户端实现)
4. [心跳机制](#心跳机制)
5. [消息广播](#消息广播)
6. [连接管理](#连接管理)
7. [错误处理](#错误处理)
8. [关键技术点](#关键技术点)

---

## 1. WebSocket 基础概念

### 1.1 什么是 WebSocket？
- **WebSocket** 是一种在单个 TCP 连接上进行全双工通信的协议
- 相比 HTTP，WebSocket 允许服务器主动向客户端推送数据
- 适用于实时通信场景，如聊天、游戏、实时数据更新等

### 1.2 WebSocket vs HTTP
| 特性 | HTTP | WebSocket |
|------|------|----------|
| 通信方式 | 请求-响应 | 全双工 |
| 连接状态 | 无状态 | 有状态 |
| 数据推送 | 需要轮询 | 服务器主动推送 |
| 开销 | 每次请求包含头部 | 握手后仅数据 |

---

## 2. 服务端实现

### 2.1 使用 gorilla/websocket 库

```go
import "github.com/gorilla/websocket"
```

### 2.2 Upgrader 配置

```go
var upgrader = websocket.Upgrader{
    CheckOrigin: func(r *http.Request) bool {
        return true // 允许所有跨域请求
    },
    ReadBufferSize:  1024,  // 读缓冲区大小
    WriteBufferSize: 1024,   // 写缓冲区大小
}
```

**关键点：**
- `CheckOrigin`: 检查跨域请求的来源（生产环境需要严格配置）
- `ReadBufferSize/WriteBufferSize`: 缓冲区大小影响性能

### 2.3 HTTP 升级为 WebSocket

```go
func HandleWebsocket(ctx *gin.Context) {
    // 将 HTTP 连接升级为 WebSocket 连接
    ws, err := upgrader.Upgrade(ctx.Writer, ctx.Request, nil)
    if err != nil {
        logrus.Printf("WebSocket升级失败:%v", err)
        ctx.AbortWithStatus(http.StatusInternalServerError)
        return
    }
    defer ws.Close() // 确保连接关闭
}
```

**知识点：**
- `Upgrade()` 方法执行 WebSocket 握手
- 握手成功后，HTTP 连接升级为 WebSocket 连接
- 必须使用 `defer` 确保连接被正确关闭

### 2.4 客户端集合管理

```go
// 全局客户端映射
var Clients = make(map[*websocket.Conn]bool)

// 注册客户端
mu.Lock()
global.Clients[ws] = true
mu.Unlock()

// 移除客户端
mu.Lock()
delete(global.Clients, ws)
mu.Unlock()
```

**关键点：**
- 使用 `map[*websocket.Conn]bool` 存储所有连接
- 必须使用互斥锁 `sync.Mutex` 保护并发访问
- 连接关闭时需要从集合中移除

### 2.5 消息读取循环

```go
for {
    mt, msg, err := ws.ReadMessage()
    if err != nil {
        // 处理错误并断开连接
        mu.Lock()
        delete(global.Clients, ws)
        mu.Unlock()
        break
    }
    
    // 处理消息
    // mt: 消息类型（TextMessage/BinaryMessage）
    // msg: 消息内容
}
```

**消息类型：**
- `websocket.TextMessage` (1): 文本消息
- `websocket.BinaryMessage` (2): 二进制消息
- `websocket.CloseMessage` (8): 关闭消息
- `websocket.PingMessage` (9): Ping 消息
- `websocket.PongMessage` (10): Pong 消息

---

## 3. 客户端实现

### 3.1 前端 WebSocket API

```javascript
// 创建 WebSocket 连接
this.ws = new WebSocket('ws://' + window.location.host + '/ws');

// 连接状态
this.ws.readyState
// CONNECTING (0): 连接中
// OPEN (1): 已连接
// CLOSING (2): 正在关闭
// CLOSED (3): 已关闭
```

### 3.2 事件处理

```javascript
// 连接打开
this.ws.onopen = function() {
    console.log('连接已建立');
    // 初始化心跳
    self.startHeartbeat();
};

// 接收消息
this.ws.onmessage = function(event) {
    var msg = JSON.parse(event.data);
    // 处理消息
};

// 连接关闭
this.ws.onclose = function(event) {
    console.log('连接已关闭', event.code, event.reason);
    // 尝试重连
};

// 连接错误
this.ws.onerror = function(err) {
    console.error('WebSocket错误:', err);
};
```

### 3.3 发送消息

```javascript
// 发送文本消息
this.ws.send(JSON.stringify({
    message: {
        userid: 'web-' + Date.now(),
        userphone: '网页-' + this.username,
        message: this.newMsg,
        time: curTime
    }
}));

// 发送心跳
this.ws.send('ping');
```

---

## 4. 心跳机制

### 4.1 心跳的作用
- **检测连接存活**: 及时发现断开的连接
- **保持连接活跃**: 防止中间设备（代理、防火墙）断开空闲连接
- **资源清理**: 及时移除无效连接

### 4.2 服务端心跳实现

```go
// 心跳超时时间
heartbeatTimeout = 60 * time.Second

// 设置读超时
ws.SetReadDeadline(time.Now().Add(heartbeatTimeout))

// 心跳重置函数
resetHeartbeat := func() {
    if err := ws.SetReadDeadline(time.Now().Add(heartbeatTimeout)); err != nil {
        // 失败时断开连接
        delete(global.Clients, ws)
        ws.Close()
    }
}

// 处理心跳包
if mt == websocket.TextMessage {
    if string(msg) == "ping" {
        // 回复 pong
        ws.WriteMessage(websocket.TextMessage, []byte("pong"))
        resetHeartbeat() // 重置超时
        continue // 不广播心跳包
    }
}
```

**关键点：**
- `SetReadDeadline()`: 设置读操作的超时时间
- 收到 `ping` 时回复 `pong`，并重置超时
- 超时后 `ReadMessage()` 会返回错误，触发连接清理

### 4.3 客户端心跳实现

```javascript
// 心跳间隔（30秒）
heartbeatInterval: 30000,

// 启动心跳
startHeartbeat: function() {
    var self = this;
    this.heartbeatTimer = setInterval(function() {
        if (self.ws && self.ws.readyState === WebSocket.OPEN) {
            self.ws.send('ping');
        }
    }, this.heartbeatInterval);
},

// 处理心跳回复
this.ws.onmessage = function(event) {
    if (event.data === 'pong') {
        console.log('收到心跳回复');
        return; // 不处理心跳消息
    }
    // 处理其他消息
};
```

---

## 5. 消息广播

### 5.1 广播通道设计

```go
// 全局广播通道
var Broadcast = make(chan StringMessage)

// 消息结构
type StringMessage struct {
    MessageType int    // 消息类型
    Message     []byte // 消息内容
}
```

### 5.2 消息投递

```go
// 在 HandleWebsocket 中，接收到消息后投递到广播通道
global.Broadcast <- global.StringMessage{
    MessageType: mt,
    Message:     msg,
}
```

### 5.3 广播协程

```go
// 启动后台协程处理广播
go handler.StartBroadCast()

func StartBroadCast() {
    for msg := range global.Broadcast {
        mu.Lock()
        // 遍历所有客户端发送消息
        for client := range global.Clients {
            if err := client.WriteMessage(msg.MessageType, msg.Message); err != nil {
                logrus.Errorf("WebSocket发送消息失败: %v", err)
                client.Close()
                delete(global.Clients, client)
            }
        }
        mu.Unlock()
    }
}
```

**设计优势：**
- 解耦：消息接收和广播分离
- 非阻塞：使用通道异步处理
- 并发安全：加锁保护客户端集合

---

## 6. 连接管理

### 6.1 连接状态跟踪

```javascript
// 连接状态
connectionStatus: 'disconnected', // connecting, connected, reconnecting, disconnected
```

### 6.2 自动重连机制

```javascript
// 重连配置
reconnectAttempts: 0,
maxReconnectAttempts: 5,
reconnectDelay: 1000,      // 初始延迟 1秒
maxReconnectDelay: 30000,   // 最大延迟 30秒

// 指数退避重连
attemptReconnect: function() {
    var self = this;
    
    if (self.reconnectAttempts >= self.maxReconnectAttempts) {
        console.log('已达到最大重连次数');
        return;
    }
    
    self.reconnectAttempts++;
    
    // 指数退避算法
    var delay = Math.min(
        self.reconnectDelay * Math.pow(2, self.reconnectAttempts - 1),
        self.maxReconnectDelay
    );
    
    setTimeout(function() {
        self.connect();
    }, delay);
}
```

**指数退避算法：**
- 第1次: 1秒
- 第2次: 2秒
- 第3次: 4秒
- 第4次: 8秒
- 第5次: 16秒（不超过最大值30秒）

### 6.3 连接清理

```go
// 服务端：连接关闭时清理
defer func() {
    mu.Lock()
    delete(global.Clients, ws)
    mu.Unlock()
    ws.Close()
}()
```

```javascript
// 客户端：页面关闭时清理
beforeDestroy: function() {
    this.clearTimers();
    if (this.ws) {
        this.ws.close(1000, '页面关闭'); // 1000: 正常关闭
    }
}
```

---

## 7. 错误处理

### 7.1 服务端错误处理

```go
// 连接升级错误
if err != nil {
    logrus.Printf("WebSocket升级失败:%v", err)
    ctx.AbortWithStatus(http.StatusInternalServerError)
    return
}

// 读取消息错误
if err != nil {
    if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
        logrus.Errorf("WebSocket读取消息失败: %v", err)
    } else {
        logrus.Debugf("连接已关闭: %v", err)
    }
    // 清理连接
    mu.Lock()
    delete(global.Clients, ws)
    mu.Unlock()
    break
}

// 发送消息错误
if err := client.WriteMessage(msg.MessageType, msg.Message); err != nil {
    logrus.Errorf("WebSocket发送消息失败: %v", err)
    client.Close()
    delete(global.Clients, client)
}
```

### 7.2 客户端错误处理

```javascript
// 连接错误
this.ws.onerror = function(err) {
    console.error('WebSocket错误:', err);
    self.connectionStatus = 'disconnected';
    self.clearTimers();
    
    if (!self.isReconnecting) {
        self.attemptReconnect();
    }
};

// 连接关闭处理
this.ws.onclose = function(event) {
    console.log('连接已关闭', event.code, event.reason);
    
    // 1000: 正常关闭（不需要重连）
    // 其他代码: 异常关闭（需要重连）
    if (event.code !== 1000 && !self.isReconnecting) {
        self.attemptReconnect();
    }
};
```

**WebSocket 关闭代码：**
- `1000`: 正常关闭
- `1001`: 端点离开
- `1002`: 协议错误
- `1003`: 不支持的数据类型
- `1006`: 异常关闭（没有关闭帧）

---

## 8. 关键技术点

### 8.1 并发安全

```go
// 使用互斥锁保护共享资源
var mu sync.Mutex

// 读写客户端集合时必须加锁
mu.Lock()
global.Clients[ws] = true
mu.Unlock()
```

### 8.2 消息格式

```javascript
// 聊天消息
{
    message: {
        userid: 'web-123456',
        userphone: '网页-用户名',
        message: '消息内容',
        time: '2024-01-01 12:00:00',
        filelink: ''
    }
}

// 在线用户通知
{
    online: {
        userid: 'web-123456',
        userphone: '网页-用户名'
    }
}
```

### 8.3 跨域配置

```go
// Gin CORS 中间件
r.Use(cors.New(cors.Config{
    AllowOrigins:     []string{"*"},
    AllowMethods:     []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"},
    AllowHeaders:     []string{"Origin", "Content-Type", "Accept"},
    AllowCredentials: true,
    MaxAge:           12 * time.Hour,
}))
```

### 8.4 性能优化

1. **缓冲区大小**: 根据消息大小调整 ReadBufferSize/WriteBufferSize
2. **连接池**: 合理管理连接数量，避免资源耗尽
3. **消息队列**: 使用通道缓冲，避免阻塞
4. **心跳优化**: 合理设置心跳间隔，平衡性能和资源

### 8.5 最佳实践

1. ✅ **总是使用 defer 关闭连接**
2. ✅ **使用互斥锁保护共享资源**
3. ✅ **实现心跳机制检测连接存活**
4. ✅ **实现自动重连机制**
5. ✅ **错误处理和日志记录**
6. ✅ **资源清理（定时器、连接等）**
7. ✅ **超时控制（读/写超时）**
8. ✅ **优雅关闭连接（发送关闭帧）**

---

## 9. 项目中的 WebSocket 应用场景

### 9.1 实时聊天
- 发送文本消息
- 接收广播消息
- 在线用户列表

### 9.2 文件传输通知
- 文件上传完成后，通过 WebSocket 通知所有用户
- 实时推送文件链接

### 9.3 在线状态管理
- 用户上线/下线通知
- 在线用户列表更新

---

## 10. 常见问题与解决方案

### Q1: 连接频繁断开？
**解决方案：**
- 检查心跳机制是否正确实现
- 调整心跳间隔和超时时间
- 检查网络环境（代理、防火墙）

### Q2: 消息丢失？
**解决方案：**
- 实现消息确认机制
- 使用消息队列缓冲
- 检查连接状态再发送

### Q3: 内存泄漏？
**解决方案：**
- 及时清理断开的连接
- 清理定时器
- 使用连接池限制连接数

### Q4: 并发问题？
**解决方案：**
- 使用互斥锁保护共享资源
- 使用通道进行协程间通信
- 避免在协程间直接共享可变状态

---

## 📝 总结

WebSocket 实现的核心要点：

1. **连接管理**: 正确管理客户端连接集合，及时清理无效连接
2. **心跳机制**: 保持连接活跃，检测连接状态
3. **消息广播**: 实现高效的消息广播机制
4. **错误处理**: 完善的错误处理和重连机制
5. **并发安全**: 使用锁和通道保证并发安全
6. **资源管理**: 及时清理资源，避免泄漏

以上是 LuChat 项目中 WebSocket 相关的完整知识点总结。
