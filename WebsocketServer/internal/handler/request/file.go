package request

// 文件分块上传请求
type ChunkUploadReq struct {
	FileHash    string `json:"file_hash" binding:"required"`    // 文件唯一标识
	ChunkIndex  int    `json:"chunk_index" binding:"required"`  // 块索引
	TotalChunks int    `json:"total_chunks" binding:"required"` // 总块数
	Filename    string `json:"filename" binding:"required"`     // 原始文件名
	ChunkData   []byte `json:"chunk_data" binding:"required"`   // 块数据
}

// 文件合并请求
type ChunkMergeReq struct {
	FileHash    string `json:"file_hash" binding:"required"`
	Filename    string `json:"filename" binding:"required"`
	TotalChunks int    `json:"total_chunks" binding:"required"`
}

// 断点续传检查请求
type ResumeCheckReq struct {
	FileHash    string `json:"file_hash" binding:"required"`
	Filename    string `json:"filename" binding:"required"`
	TotalChunks int    `json:"total_chunks" binding:"required"`
	FileSize    int64  `json:"file_size" binding:"required"`
}

// 断点续传检查响应
type ResumeCheckResp struct {
	FileHash       string `json:"file_hash"`
	UploadedChunks []int  `json:"uploaded_chunks"` // 已上传的分块索引列表
	TotalChunks    int    `json:"total_chunks"`
	CanResume      bool   `json:"can_resume"` // 是否可以续传
}

// 秒传检查请求
type InstantUploadReq struct {
	FileHash string `json:"file_hash" binding:"required"`
	Filename string `json:"filename" binding:"required"`
	FileSize int64  `json:"file_size" binding:"required"`
}

// 秒传检查响应
type InstantUploadResp struct {
	FileHash   string `json:"file_hash"`
	CanInstant bool   `json:"can_instant"` // 是否可以秒传
	FileExists bool   `json:"file_exists"` // 文件是否已存在
	FileUrl    string `json:"file_url"`    // 文件访问URL（如果存在）
	Message    string `json:"message"`     // 提示信息
}
