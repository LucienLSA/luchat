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

// 删除文件记录
func DeleteUploadedFile(filename string) error {
	if err := DB.Where("file_name = ?", filename).Delete(&model.ChatUploadFile{}).Error; err != nil {
		logrus.Errorf("删除文件记录失败: %v", err)
		return err
	}
	return nil
}

// 根据文件哈希查找文件
func GetFileByHash(fileHash string) (*model.ChatUploadFile, error) {
	var file model.ChatUploadFile
	if err := DB.Where("file_hash = ?", fileHash).First(&file).Error; err != nil {
		if err.Error() == "record not found" {
			return nil, nil // 文件不存在
		}
		logrus.Errorf("根据哈希查找文件失败: %v", err)
		return nil, err
	}
	return &file, nil
}

// 获取文件列表
func GetFileList() ([]model.ChatUploadFile, error) {
	var files []model.ChatUploadFile
	if err := DB.Order("created_at DESC").Find(&files).Error; err != nil {
		logrus.Errorf("获取文件列表失败: %v", err)
		return nil, err
	}
	return files, nil
}
