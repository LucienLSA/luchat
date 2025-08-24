package db

import (
	"fmt"
	"luchat/WebsocketServer/internal/model"

	"github.com/sirupsen/logrus"
)

func VerifyUser(user *model.ChatUser) error {
	if err := DB.Where("userphone=? AND password = ?", user.UserPhone,
		user.Password).First(user).Error; err != nil {
		logrus.Errorf("验证用户名密码失败: %v", err)
		return err
	}
	return nil
}

func CreateUser(user *model.ChatUser) error {
	// 先检查用户是否已存在
	if IsUserExist(user.UserPhone) {
		return fmt.Errorf("用户 %s 已存在", user.UserPhone)
	}

	if err := DB.Create(user).Error; err != nil {
		logrus.Errorf("创建用户失败: %v", err)
		return err
	}
	return nil
}

// 检查用户是否存在
func IsUserExist(userPhone string) bool {
	var count int64
	if err := DB.Model(&model.ChatUser{}).Where("userphone = ?", userPhone).Count(&count).Error; err != nil {
		logrus.Errorf("检查用户是否存在失败: %v", err)
		return false
	}
	return count > 0
}

// 检查用户是否存在（返回错误信息）
func CheckUserExists(userPhone string) error {
	if IsUserExist(userPhone) {
		return fmt.Errorf("用户 %s 已存在", userPhone)
	}
	return nil
}

// // 初始化测试用户
// func InitTestUser() {
// 	// 检查是否已存在测试用户
// 	var count int64
// 	DB.Model(&model.ChatUser{}).Where("userphone = ?", "test").Count(&count)
// 	if count == 0 {
// 		// 创建测试用户
// 		testUser := &model.ChatUser{
// 			UserPhone: "test",
// 			Password:  "123456",
// 		}
// 		if err := DB.Create(testUser).Error; err != nil {
// 			logrus.Errorf("创建测试用户失败: %v", err)
// 		} else {
// 			logrus.Info("测试用户创建成功: test/123456")
// 		}
// 	}
// }
