package handler

import (
	"fmt"
	"io"
	"luchat/WebsocketServer/internal/global"
	"luchat/WebsocketServer/internal/handler/request"
	"luchat/WebsocketServer/internal/handler/response"
	"luchat/WebsocketServer/internal/service"
	"mime"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
)

func UploadFile(c *gin.Context) {
	// 表单获取文件
	file, err := c.FormFile("file")
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"msg": "文件获取失败:" + err.Error(),
		})
		return
	}
	// 安全处理文件名：移除路径分隔符，防止路径遍历攻击
	filename := strings.ReplaceAll(file.Filename, "/", "")
	filename = strings.ReplaceAll(filename, "\\", "")
	saveDir := filepath.Join("public", "uploads")
	if err := os.MkdirAll(saveDir, 0755); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"msg": "创建上传目录失败：" + err.Error(),
		})
		return
	}
	savePath := filepath.Join(saveDir, filename)
	if err := c.SaveUploadedFile(file, savePath); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"msg": "文件保存失败：" + err.Error(),
		})
		return
	}
	// 保存到数据库
	uploadUser := c.PostForm("userphone")
	if err := service.SaveUploadedFile(filename, file.Size, uploadUser); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"msg": "保存上传记录失败" + err.Error(),
		})
		return
	}
	// 后续可保存到OSS
	c.JSON(http.StatusOK, gin.H{
		"msg":       "上传成功",
		"file_path": "/public/uploads/" + filename,
	})
}

// 校验文件类型是否允许
func checkFileType(contentType string) bool {
	allowed := strings.Split(global.AllowedTypes, ",")
	for _, t := range allowed {
		if t == contentType {
			return true
		}
	}
	return false
}

// 上传文件块
func UploadChunk(c *gin.Context) {
	// 获取表单数据
	fileHash := c.PostForm("file_hash")
	chunkIndexStr := c.PostForm("chunk_index")
	totalChunksStr := c.PostForm("total_chunks")
	filename := c.PostForm("filename")

	if fileHash == "" || chunkIndexStr == "" || totalChunksStr == "" || filename == "" {
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 验证文件类型
	fileExt := filepath.Ext(filename)
	mimeType := mime.TypeByExtension(fileExt)
	if !checkFileType(mimeType) {
		response.ResponseError(c, response.CodeInvalidFileType)
		return
	}

	// 获取分块文件
	file, err := c.FormFile("chunk_data")
	if err != nil {
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 创建临时存储目录
	tempDir := filepath.Join("public", "temp", fileHash)
	if err := os.MkdirAll(tempDir, 0755); err != nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	// 保存块文件
	chunkPath := filepath.Join(tempDir, chunkIndexStr)
	if err := c.SaveUploadedFile(file, chunkPath); err != nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	// 返回分块文件上传保存
	response.ResponseSuccessData(c, gin.H{"status": "chunk saved"})
}

// 检查断点续传
func CheckResume(c *gin.Context) {
	var req request.ResumeCheckReq
	if err := c.ShouldBindJSON(&req); err != nil {
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 验证文件类型
	fileExt := filepath.Ext(req.Filename)
	mimeType := mime.TypeByExtension(fileExt)
	if !checkFileType(mimeType) {
		response.ResponseError(c, response.CodeInvalidFileType)
		return
	}

	// 检查临时目录是否存在
	tempDir := filepath.Join("public", "temp", req.FileHash)
	var uploadedChunks []int

	// 检查已上传的分块
	for i := 0; i < req.TotalChunks; i++ {
		chunkPath := filepath.Join(tempDir, fmt.Sprintf("%d", i))
		if _, err := os.Stat(chunkPath); err == nil {
			uploadedChunks = append(uploadedChunks, i)
		}
	}

	// 判断是否可以续传
	canResume := len(uploadedChunks) > 0 && len(uploadedChunks) < req.TotalChunks

	resp := request.ResumeCheckResp{
		FileHash:       req.FileHash,
		UploadedChunks: uploadedChunks,
		TotalChunks:    req.TotalChunks,
		CanResume:      canResume,
	}

	response.ResponseSuccessData(c, resp)
}

// 检查秒传
func CheckInstantUpload(c *gin.Context) {
	var req request.InstantUploadReq
	if err := c.ShouldBindJSON(&req); err != nil {
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 验证文件类型
	fileExt := filepath.Ext(req.Filename)
	mimeType := mime.TypeByExtension(fileExt)
	if !checkFileType(mimeType) {
		response.ResponseError(c, response.CodeInvalidFileType)
		return
	}

	// 检查数据库中是否存在相同哈希的文件
	existingFile, err := service.GetFileByHash(req.FileHash)
	if err != nil {
		logrus.Errorf("查询文件哈希失败: %v", err)
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	resp := request.InstantUploadResp{
		FileHash:   req.FileHash,
		CanInstant: false,
		FileExists: false,
		FileUrl:    "",
		Message:    "",
	}

	if existingFile != nil {
		// 文件已存在，可以秒传
		resp.CanInstant = true
		resp.FileExists = true
		resp.FileUrl = "/api/download?filename=" + existingFile.FileName
		resp.Message = "文件已存在，秒传成功"

		// 更新文件的上传用户（如果是新用户上传）
		uploadUser := c.PostForm("userphone")
		if uploadUser != "" && uploadUser != existingFile.UploadUser {
			// 可以选择更新上传用户或者创建新的记录
			// 这里我们选择创建新记录，表示该用户也上传了这个文件
			service.SaveUploadedFile(req.Filename, req.FileSize, uploadUser)
		}
	} else {
		resp.Message = "文件不存在，需要上传"
	}

	response.ResponseSuccessData(c, resp)
}

// 合并文件块
func MergeChunks(c *gin.Context) {
	// 表单验证
	var req request.ChunkMergeReq
	if err := c.ShouldBindJSON(&req); err != nil {
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}
	// 获取临时存储分块文件路径
	tempDir := filepath.Join("public", "temp", req.FileHash)
	// 获取目标存储文件路径
	saveDir := filepath.Join("public", "uploads")
	if err := os.MkdirAll(saveDir, 0755); err != nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	// 验证所有块是否齐全
	for i := 0; i < req.TotalChunks; i++ {
		chunkPath := filepath.Join(tempDir, fmt.Sprintf("%d", i))
		if _, err := os.Stat(chunkPath); os.IsNotExist(err) {
			response.ResponseErrorMsg(c, response.CodeInvalidParam, "缺少文件块")
			return
		}
	}

	// 合并文件
	dstPath := filepath.Join(saveDir, req.Filename)
	dstFile, err := os.Create(dstPath)
	if err != nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}
	defer dstFile.Close()

	// 按顺序写入所有块
	for i := 0; i < req.TotalChunks; i++ {
		chunkPath := filepath.Join(tempDir, fmt.Sprintf("%d", i))
		chunkFile, err := os.Open(chunkPath)
		if err != nil {
			response.ResponseError(c, response.CodeServerBusy)
			return
		}

		if _, err := io.Copy(dstFile, chunkFile); err != nil {
			chunkFile.Close()
			response.ResponseError(c, response.CodeServerBusy)
			return
		}
		chunkFile.Close()
		os.Remove(chunkPath) // 删除临时块文件
	}

	// 删除临时目录
	os.RemoveAll(tempDir)

	// 保存文件记录到数据库（带文件哈希）
	uploadUser := c.PostForm("userphone")
	fileInfo, _ := os.Stat(dstPath)
	if err := service.SaveUploadedFileWithHash(req.Filename, fileInfo.Size(), req.FileHash, uploadUser); err != nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}

	response.ResponseSuccessData(c, gin.H{
		"msg":       "文件上传成功",
		"file_path": "/public/uploads/" + req.Filename,
	})
}

// 下载文件
func DownloadFile(c *gin.Context) {
	filename := c.Query("filename")
	if filename == "" {
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 安全处理文件名
	filename = strings.ReplaceAll(filename, "/", "")
	filename = strings.ReplaceAll(filename, "\\", "")
	filePath := filepath.Join("public", "uploads", filename)

	// 检查文件是否存在
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		response.ResponseErrorMsg(c, response.CodeInvalidParam, "文件不存在")
		return
	}

	// 设置响应头
	c.Header("Content-Disposition", fmt.Sprintf("attachment; filename=%s", filename))
	c.Header("Content-Type", "application/octet-stream")

	// 传输文件
	c.File(filePath)
}

// 实现删除文件方法
func DeleteFile(c *gin.Context) {
	filename := c.Query("filename")
	logrus.Printf("收到删除文件请求: %s", filename)

	if filename == "" {
		logrus.Error("文件名参数为空")
		response.ResponseError(c, response.CodeInvalidParam)
		return
	}

	// 安全处理文件名
	filename = strings.ReplaceAll(filename, "/", "")
	filename = strings.ReplaceAll(filename, "\\", "")
	filePath := filepath.Join("public", "uploads", filename)
	logrus.Printf("文件路径: %s", filePath)

	// 检查文件是否存在
	if _, err := os.Stat(filePath); os.IsNotExist(err) {
		logrus.Errorf("文件不存在: %s", filePath)
		response.ResponseErrorMsg(c, response.CodeInvalidParam, "文件不存在")
		return
	}

	// 删除文件
	if err := os.Remove(filePath); err != nil {
		logrus.Errorf("删除文件失败: %v", err)
		response.ResponseError(c, response.CodeServerBusy)
		return
	}
	logrus.Printf("文件删除成功: %s", filePath)

	// 从数据库删除记录
	if err := service.DeleteUploadedFile(filename); err != nil {
		logrus.Errorf("删除数据库记录失败: %v", err)
		response.ResponseError(c, response.CodeServerBusy)
		return
	}
	logrus.Printf("数据库记录删除成功: %s", filename)

	response.ResponseSuccessData(c, gin.H{"msg": "文件删除成功"})
}

// 获取文件列表
func GetFileList(c *gin.Context) {
	files, err := service.GetFileList()
	if err != nil {
		response.ResponseError(c, response.CodeServerBusy)
		return
	}
	response.ResponseSuccessData(c, gin.H{"files": files})
}
