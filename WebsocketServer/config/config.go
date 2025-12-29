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

type AIConfig struct {
	APIKey      string  `json:"api_key"`
	BaseURL     string  `json:"base_url"`
	Model       string  `json:"model"`
	Temperature float32 `json:"temperature"`
	MaxTokens   int     `json:"max_tokens"`
	Timeout     int     `json:"timeout"` // 秒
}

type Config struct {
	Database SqlConfig `json:"database"`
	AI       AIConfig  `json:"ai"`
}

// 全局配置实例
var Cfg Config

func ReadConfig(path string) Config {
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
