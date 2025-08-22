package global

import "database/sql"

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
