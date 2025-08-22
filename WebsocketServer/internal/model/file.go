package model

import (
	"time"

	"gorm.io/gorm"
)

type ChatUploadFile struct {
	ID         int            `gorm:"column:id;primaryKey;autoIncrement" json:"id" comment:"文件ID"`
	FileName   string         `gorm:"column:file_name" json:"file_name" comment:"文件名"`
	FileSize   int64          `gorm:"column:file_size" json:"file_size" comment:"文件大小"`
	UploadUser string         `gorm:"column:upload_user" json:"upload_user" comment:"上传用户"`
	CreatedAt  time.Time      `gorm:"column:created_at;autoCreateTime" json:"created_at" comment:"创建时间"` // 自动管理时间
	UpdatedAt  *time.Time     `gorm:"column:updated_at;autoUpdateTime" json:"updated_at" comment:"更新时间"`
	DeletedAt  gorm.DeletedAt `gorm:"column:deleted_at" json:"deleted_at" comment:"删除时间"`
}

func (ChatUploadFile) TableName() string {
	return "chat_upload_file"
}
