package db

import (
	"luchat/WebsocketServer/internal/model"

	"github.com/sirupsen/logrus"
)

func VerifyUser(user *model.ChatUser) error {
	if err := DB.Where("username=? AND password = ?", user.Username,
		user.Password).First(user).Error; err != nil {
		logrus.Errorf("验证用户名密码失败: %v", err)
		return err
	}
	return nil
}

func CreateUser(user *model.ChatUser) error {
	if err := DB.Create(user).Error; err != nil {
		logrus.Errorf("创建用户失败: %v", err)
		return err
	}
	return nil
}
