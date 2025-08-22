package db

import (
	"luchat/WebsocketServer/internal/model"

	"github.com/sirupsen/logrus"
)

func SaveUploadedFile(file *model.ChatUploadFile) error {
	if err := DB.Create(file).Error; err != nil {
		logrus.Errorf("保存文件记录失败: %v, 文件信息: %+v", err, file)
		return err
	}
	return nil
}
