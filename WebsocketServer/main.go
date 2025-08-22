package main

import (
	"luchat/WebsocketServer/config"
	"luchat/WebsocketServer/internal/db"
	"luchat/WebsocketServer/internal/handler"
	"luchat/WebsocketServer/internal/router"
	"luchat/WebsocketServer/pkg/logger"
	"os"

	"github.com/sirupsen/logrus"
)

func main() {
	// 初始化配置
	config.ReadConfig("./config/config.json")
	// 初始化日志
	logger.Init("./log", "WebSocketServer.log")
	// 初始化数据库
	if err := db.InitMySQL(config.Cfg); err != nil {
		logrus.Fatalf("数据库初始化失败: %v", err)
	}
	// 初始化Gin Route
	r := router.Init()
	// 广播协程
	go handler.StartBroadCast()

	// 启动服务
	port := "5133"
	if len(os.Args) >= 3 {
		port = os.Args[2]
	}
	logrus.Printf("服务启动端口:%s", port)
	if err := r.Run(":" + port); err != nil {
		logrus.Fatalf("服务启动失败:%v", err)
	}

}
