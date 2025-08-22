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
	r.Static("/public", "./public")
	r.Static("/fonts", "./public/fonts")
	r.StaticFile("/", "./public/index.html")
	//  接口
	api := r.Group("/api")
	{
		api.POST("/login", handler.Login)
		api.POST("/register", handler.Register) // 后续实现
		api.POST("/upload", handler.UploadFile)

	}
	// WebSocket路由
	r.GET("/ws", handler.HandleWebsocket)
	return r
}
