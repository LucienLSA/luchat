package service

import (
	"luchat/WebsocketServer/internal/db"
	"luchat/WebsocketServer/internal/model"
)

// 验证用户
func VerifyUser(username, password string) (*model.ChatUser, error) {
	user := model.ChatUser{Username: username, Password: password}
	if err := db.VerifyUser(&user); err != nil {
		return nil, err
	}
	return &user, nil
}

// 新增用户
func CreateUser(username, password string) (*model.ChatUser, error) {
	user := model.ChatUser{Username: username, Password: password}
	if err := db.CreateUser(&user); err != nil {
		return nil, err
	}
	return &user, nil
}
