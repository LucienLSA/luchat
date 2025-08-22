package model

import (
	"time"

	"gorm.io/gorm"
)

type ChatClient struct {
	ID          int            `gorm:"column:id;primaryKey;autoIncrement" json:"id" comment:"客户端ID"`
	Version     string         `gorm:"column:version" json:"version" comment:"客户端版本"`
	VersionName string         `gorm:"column:version_name" json:"version_name" comment:"客户端版本名称"`
	FileName    string         `gorm:"column:file_name" json:"file_name" comment:"文件名"`
	FileSize    int64          `gorm:"column:file_size" json:"file_size" comment:"文件大小"`
	MD5         string         `gorm:"column:md5" json:"md5" comment:"文件MD5"`
	UploadTime  time.Time      `gorm:"column:upload_time" json:"upload_time" comment:"上传时间"`
	UploadBy    string         `gorm:"column:upload_by" json:"upload_by" comment:"上传用户"`
	DeletedAt   gorm.DeletedAt `gorm:"column:deleted_at" json:"deleted_at" comment:"删除时间"`
}

func (ChatClient) TableName() string {
	return "chat_client"
}
