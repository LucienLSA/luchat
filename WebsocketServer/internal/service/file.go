package service

import (
	"luchat/WebsocketServer/internal/db"
	"luchat/WebsocketServer/internal/model"
	"time"
)

// 记录上传文件到数据库
func SaveUploadedFile(filename string, filesize int64, uploadUser string) error {
	file := model.ChatUploadFile{
		FileName:   filename,
		FileSize:   filesize,
		UploadUser: uploadUser,
		CreatedAt:  time.Now(),
	}
	return db.SaveUploadedFile(&file)
}

// 记录上传文件到数据库（带哈希）
func SaveUploadedFileWithHash(filename string, filesize int64, fileHash string, uploadUser string) error {
	file := model.ChatUploadFile{
		FileName:   filename,
		FileSize:   filesize,
		FileHash:   fileHash,
		UploadUser: uploadUser,
		CreatedAt:  time.Now(),
	}
	return db.SaveUploadedFile(&file)
}

// 根据文件哈希查找文件
func GetFileByHash(fileHash string) (*model.ChatUploadFile, error) {
	return db.GetFileByHash(fileHash)
}

// 删除文件记录
func DeleteUploadedFile(filename string) error {
	return db.DeleteUploadedFile(filename)
}

// 获取文件列表
func GetFileList() ([]model.ChatUploadFile, error) {
	return db.GetFileList()
}
