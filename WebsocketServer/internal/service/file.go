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
