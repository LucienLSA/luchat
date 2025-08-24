package db

import (
	"fmt"
	"luchat/WebsocketServer/config"
	"luchat/WebsocketServer/internal/model"

	"github.com/sirupsen/logrus"
	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

var DB *gorm.DB

func InitMySQL(cfg config.SqlConfig) error {
	var err error
	dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s?charset=%s&parseTime=True&loc=Local", cfg.UserName, cfg.Password, cfg.Host, cfg.Port, cfg.Database, cfg.Charset)
	DB, err = gorm.Open(mysql.Open(dsn), &gorm.Config{})
	if err != nil {
		logrus.Panicf("数据库连接失败:%s", err.Error())
		return err
	}
	return DB.AutoMigrate(
		&model.ChatUser{},
		&model.ChatUploadFile{},
		&model.ChatClient{},
	)
}
