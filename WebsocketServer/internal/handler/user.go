package handler

import (
	"luchat/WebsocketServer/internal/handler/request"
	"luchat/WebsocketServer/internal/service"
	"net/http"

	"github.com/gin-gonic/gin"
)

func Login(c *gin.Context) {
	var req request.LoginReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"msg": "参数错误:" + err.Error(),
		})
		return
	}
	// 验证用户
	user, err := service.VerifyUser(req.UserName, req.Password)
	if err != nil {
		c.JSON(http.StatusOK, gin.H{
			"msg": "登录失败:" + err.Error(),
		})
		return
	}
	c.JSON(http.StatusOK, gin.H{
		"msg":      "登录成功",
		"userid":   user.ID,
		"username": user.Username,
	})
}

func Register(c *gin.Context) {
	var req request.RegisterReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"msg": "参数错误:" + err.Error(),
		})
		return
	}
	// 验证用户
	user, err := service.CreateUser(req.UserName, req.Password)
	if err != nil {
		c.JSON(http.StatusOK, gin.H{
			"msg": "注册失败:" + err.Error(),
		})
		return
	}
	c.JSON(http.StatusOK, gin.H{
		"msg":      "注册成功",
		"userid":   user.ID,
		"username": user.Username,
	})
}
