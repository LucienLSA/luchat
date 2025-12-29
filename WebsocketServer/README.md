# WebSocket 聊天服务器

这是一个基于 Go 和 Gin 框架的 WebSocket 聊天服务器项目，支持实时聊天、文件上传等功能。

## 项目结构

```
.
├── cmd/                    # 应用程序入口
│   └── server/            # 服务器主程序
│       └── main.go
├── config/                 # 配置文件
│   ├── config.go
│   └── config.json.tpl
├── docs/                   # 文档
│   └── WebSocket知识点总结.md
├── internal/               # 私有应用代码
│   ├── db/                # 数据库相关
│   ├── global/            # 全局变量
│   ├── handler/           # HTTP 处理器
│   ├── model/             # 数据模型
│   ├── router/            # 路由配置
│   └── service/           # 业务逻辑
├── pkg/                    # 可复用的包
│   └── logger/            # 日志包
├── sql/                    # 数据库表结构
├── web/                    # 前端静态文件
├── go.mod                  # Go 模块定义
└── go.sum                  # Go 依赖校验和
```

## 功能特性

- WebSocket 实时聊天
- 用户注册和登录
- 文件上传和下载
- 分块上传和断点续传
- 消息广播
- 🤖 AI对话功能（集成豆包AI）

## 快速开始

### 环境要求

- Go 1.19+
- MySQL 5.7+

### 安装和运行

1. 克隆项目
```bash
git clone <repository-url>
cd WebsocketServer
```

2. 安装依赖
```bash
go mod tidy
```

3. 配置数据库
- 创建 MySQL 数据库
- 执行 `sql/` 目录下的 SQL 文件创建表结构
- 修改 `config/config.json` 中的数据库配置

4. 配置AI功能
- 编辑 `config/config.json`，填入豆包AI的API密钥和其他配置
- 从[豆包AI控制台](https://console.volcengine.com/ark/)获取API密钥

5. 构建和运行
```bash
go build ./cmd/server
./server
```

服务器将在端口 5133 启动。

## API 接口

### 用户相关
- `POST /api/login` - 用户登录
- `POST /api/register` - 用户注册

### 文件相关
- `POST /api/upload` - 文件上传
- `DELETE /api/deletefile` - 删除文件
- `POST /api/chunk` - 上传分块
- `POST /api/merge` - 合并分块
- `POST /api/resume/check` - 检查断点续传
- `POST /api/instant/check` - 检查秒传
- `GET /api/download` - 下载文件
- `GET /api/files` - 获取文件列表

### AI对话相关
- `POST /api/ai/chat` - AI对话
- `GET /api/ai/health` - AI服务健康检查

### WebSocket
- `GET /ws` - WebSocket 连接

更多接口详情请查看 `internal/router/route.go`

## 前端访问

打开浏览器访问 `http://localhost:5133` 即可使用聊天界面。

## AI对话功能使用

### WebSocket聊天中的AI对话

#### 方法1：使用指令
在聊天框输入以下命令与AI对话：

```
/ai 你好，请介绍一下自己
```

或

```
@ai 今天天气怎么样？
```

#### 方法2：使用AI按钮
1. 在输入框输入问题
2. 点击蓝色的🤖按钮直接发送给AI

AI回复会通过WebSocket广播给所有在线用户，并有特殊的视觉标识（蓝色渐变背景）。

### HTTP API调用AI

也可以通过HTTP API直接调用AI：

```bash
curl -X POST http://localhost:5133/api/ai/chat \
  -H "Content-Type: application/json" \
  -d '{
    "message": "你好，请介绍一下字节跳动",
    "stream": false
  }'
```

响应示例：
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "message": "我是豆包，由字节跳动开发的AI助手...",
    "conversation_id": "uuid-string",
    "finish_reason": "stop"
  }
}
```

### 前端AI功能特色

#### 视觉体验
- 🤖 **AI消息特殊样式**: 蓝色渐变背景，机器人图标标识
- 💭 **思考状态显示**: AI回复前显示"正在思考..."提示
- 🎯 **智能按钮**: 蓝色机器人按钮，悬停动画效果
- 💡 **使用提示**: 输入框显示AI使用说明

#### 交互体验
- ⚡ **快捷指令**: 支持 `@ai` 和 `/ai` 前缀
- 🚀 **一键发送**: 点击按钮直接发送给AI
- 🔄 **状态管理**: 防止重复发送，智能状态控制
- 📱 **响应式设计**: 适配不同屏幕尺寸

### AI配置说明

在 `config/config.json` 中的AI配置项：

```json
{
  "ai": {
    "api_key": "your-doubao-api-key",
    "base_url": "https://ark.cn-beijing.volces.com/api/v3",
    "model": "doubao-pro-32k",
    "temperature": 0.7,
    "max_tokens": 2048,
    "timeout": 30
  }
}
```

配置项说明：
- `api_key`: 豆包AI的API密钥
- `base_url`: API基础URL
- `model`: 使用的AI模型
- `temperature`: 温度参数（0.0-1.0，影响随机性）
- `max_tokens`: 最大token数
- `timeout`: 请求超时时间（秒）

## 开发

### 项目结构说明

遵循 Go 项目标准布局：
- `cmd/` 存放主程序
- `internal/` 存放私有应用代码
- `pkg/` 存放可复用的包
- `web/` 存放前端静态文件

### 添加新功能

1. 在 `internal/handler/` 中添加处理器
2. 在 `internal/router/route.go` 中注册路由
3. 如需要，在 `internal/service/` 中添加业务逻辑
4. 在 `internal/model/` 中定义数据结构

## 许可证

[请添加许可证信息]
