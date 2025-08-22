package config

import (
	"encoding/json"
	"os"
)

type SqlConfig struct {
	Host     string `json:"host"`
	Port     int    `json:"port"`
	Database string `json:"database"`
	UserName string `json:"username"`
	Password string `json:"password"`
	Charset  string `json:"charset"`
}

// 全局配置实例
var Cfg SqlConfig

func ReadConfig(path string) SqlConfig {
	data, err := os.ReadFile(path)
	if err != nil {
		panic("read config file failed:" + err.Error())
	}
	err = json.Unmarshal(data, &Cfg)
	if err != nil {
		panic("parse config file failed:" + err.Error())
	}
	return Cfg
}
