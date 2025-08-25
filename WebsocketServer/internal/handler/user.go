package handler

import (
	"luchat/WebsocketServer/internal/handler/request"
	"luchat/WebsocketServer/internal/handler/response"
	"luchat/WebsocketServer/internal/service"

	"github.com/gin-gonic/gin"
)

func Login(c *gin.Context) {
	var req request.LoginReq
	if err := c.ShouldBindJSON(&req); err != nil {
		// c.JSON(http.StatusBadRequest, gin.H{
		// 	"code": 401,
		// 	"msg":  "参数错误:" + err.Error(),
		// })
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 验证用户
	user, err := service.VerifyUser(req.UserPhone, req.Password)
	if err != nil {
		// c.JSON(http.StatusOK, gin.H{
		// 	"code": 502,
		// 	"msg":  "登录失败:" + err.Error(),
		// })
		if err == response.ErrorUserNotExist {
			response.ResponseError(c, response.CodeUserNotExist)
		} else if err == response.ErrorInvalid {
			response.ResponseError(c, response.CodeInvalidPassword)
		} else {
			response.ResponseError(c, response.CodeLoginFailed)
		}
		return
	}
	// c.JSON(http.StatusOK, gin.H{
	// 	"code":      200,
	// 	"msg":       "登录成功",
	// 	"userid":    user.ID,
	// 	"userphone": user.UserPhone,
	// })
	userInfo := &response.UserLoginResp{
		UserID:    user.ID,
		UserPhone: user.UserPhone,
	}
	response.ResponseSuccessData(c, userInfo)
}

func Register(c *gin.Context) {
	var req request.RegisterReq
	if err := c.ShouldBindJSON(&req); err != nil {
		// c.JSON(http.StatusBadRequest, gin.H{
		// 	"code": 401,
		// 	"msg":  "参数错误:" + err.Error(),
		// })
		// return
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 验证用户
	user, err := service.CreateUser(req.UserPhone, req.Password)
	if err != nil {
		if err == response.ErrorUserExist {
			response.ResponseError(c, response.CodeUserExist)
			return
		} else {
			response.ResponseError(c, response.CodeServerBusy)
		}
		return
	}

	// c.JSON(http.StatusOK, gin.H{
	// 	"code":      200,
	// 	"msg":       "注册成功",
	// 	"userid":    user.ID,
	// 	"userphone": user.UserPhone,
	// })
	userInfo := &response.UserRegisterResp{
		UserID:    user.ID,
		UserPhone: user.UserPhone,
	}
	response.ResponseSuccessData(c, userInfo)
}
