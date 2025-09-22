package global

import "database/sql"

// 文件上传配置
const (
	MaxFileSize  = 1024 * 1024 * 100 // 100MB
	AllowedTypes = "image/jpeg,image/png,application/pdf,application/zip"
	ChunkSize    = 1024 * 1024 * 5 // 5MB每块
)

// 文件相关
type FileInfo struct {
	FileName   string
	FileSize   int
	UploadUser sql.NullString
}

// 文件操作
type DeleteFile struct {
	FileName string
}

type DeleteFile2 struct {
	UserName string
	FileName string
}
