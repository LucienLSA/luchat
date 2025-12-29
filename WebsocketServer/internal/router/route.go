package router

import (
	"luchat/WebsocketServer/internal/handler"
	"luchat/WebsocketServer/pkg/logger"
	"time"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
)

func Init() *gin.Engine {
	r := gin.New()
	r.Use(logger.GinLogger())
	// 4. 跨域配置（替代原CheckOrigin）
	r.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"*"},
		AllowMethods:     []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"},
		AllowHeaders:     []string{"Origin", "Content-Type", "Accept"},
		AllowCredentials: true,
		MaxAge:           12 * time.Hour,
	}))
	// 5. 静态资源
	r.Static("/public", "./web")
	r.Static("/fonts", "./web/fonts")
	r.StaticFile("/", "./web/index.html")
	//  接口
	api := r.Group("/api")
	{
		api.POST("/login", handler.Login)
		api.POST("/register", handler.Register)

		api.POST("/upload", handler.UploadFile)
		api.DELETE("/deletefile", handler.DeleteFile)
		api.POST("/chunk", handler.UploadChunk)                // 上传分块
		api.POST("/merge", handler.MergeChunks)                // 合并分块
		api.POST("/resume/check", handler.CheckResume)         // 检查断点续传
		api.POST("/instant/check", handler.CheckInstantUpload) // 检查秒传
		api.GET("/download", handler.DownloadFile)             // 下载文件
		api.GET("/files", handler.GetFileList)                 // 获取文件列表

		// AI对话相关接口
		api.POST("/ai/chat", handler.AIChat)         // AI对话
		api.GET("/ai/health", handler.AIHealthCheck) // AI服务健康检查

	}
	// WebSocket路由
	r.GET("/ws", handler.HandleWebsocket)
	return r
}
