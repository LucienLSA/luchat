package service

import (
	"luchat/WebsocketServer/internal/db"
	"luchat/WebsocketServer/internal/handler/response"
	"luchat/WebsocketServer/internal/model"
)

// 验证用户
func VerifyUser(userphone, password string) (*model.ChatUser, error) {
	if ok := db.IsUserExist(userphone); !ok {
		return nil, response.ErrorUserNotExist
	}
	user := model.ChatUser{UserPhone: userphone, Password: password}
	if err := db.VerifyUser(&user); err != nil {
		return nil, response.ErrorInvalid
	}
	return &user, nil
}

// 新增用户
func CreateUser(userphone, password string) (*model.ChatUser, error) {
	// 检验当前用户手机号是否已经注册过
	if err := db.CheckUserExists(userphone); err != nil {
		return nil, response.ErrorUserExist
	}

	user := model.ChatUser{UserPhone: userphone, Password: password}
	if err := db.CreateUser(&user); err != nil {
		return nil, response.ErrorServerBusy
	}
	return &user, nil
}
