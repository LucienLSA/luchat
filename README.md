# LuChat
基于 Go 语言的 WebSocket 服务器和 Qt 框架的 WebSocket 客户端的聊天项目 

来源：https://github.com/waynecn/WebSocketServerAndClient

该项目仅用于个人用途，严禁商业使用，如需转载请注明来源。


# 主要功能
基础聊天：客户端与服务器通过 WebSocket 进行实时消息交互，支持文本消息发送与接收。
用户相关：包含用户登录、注册功能，服务器会对用户信息进行数据库存储（使用 chat_user 表）。
文件操作：
支持文件上传到服务器，服务器将文件存储在 public/uploads 目录，并在 chat_upload_files 表记录文件信息。
可查询已上传文件列表，也能删除指定文件。
客户端版本管理：支持客户端上传新版本，服务器在 easy_chat_client 表记录相关版本信息（版本号、文件名、MD5 值等）。
日志记录：服务器使用 logrus 结合日志轮转工具记录运行日志，便于问题排查。

# 技术栈
服务器：Go 语言，使用 gorilla/websocket 处理 WebSocket 连接，go-sql-driver/mysql 和 mattn/go-sqlite3 处理数据库操作，logrus 用于日志记录等。
客户端：Qt 框架（C++），使用 QWebSocket 进行 WebSocket 通信，QNetworkAccessManager 处理 HTTP 相关请求（如文件上传）等。

# 其他说明

服务器支持通过命令行参数指定端口，默认端口为 5133。
客户端提供了设置服务器 IP 和端口的功能，若配置发生改变可能需要重启程序。