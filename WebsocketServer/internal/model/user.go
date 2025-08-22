package model

import (
	"time"

	"gorm.io/gorm"
)

type ChatUser struct {
	ID        int            `gorm:"column:id;primaryKey;autoIncrement" json:"id" comment:"用户ID"`
	Username  string         `gorm:"column:username" json:"username" comment:"用户名"`
	Password  string         `gorm:"column:password" json:"-" comment:"密码"`
	CreatedAt time.Time      `gorm:"column:created_at" json:"created_at" comment:"创建时间"`
	UpdatedAt *time.Time     `gorm:"column:updated_at" json:"updated_at" comment:"更新时间"`
	DeletedAt gorm.DeletedAt `gorm:"column:deleted_at" json:"deleted_at" comment:"删除时间"`
}

func (ChatUser) TableName() string {
	return "chat_user"
}
