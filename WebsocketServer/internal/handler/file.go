package handler

import (
	"luchat/WebsocketServer/internal/service"
	"net/http"
	"path/filepath"
	"strings"

	"github.com/gin-gonic/gin"
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
	savePath := filepath.Join("public", "uploads", filename)
	if err := c.SaveUploadedFile(file, savePath); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"msg": "文件保存失败：" + err.Error(),
		})
		return
	}
	// 保存到数据库
	uploadUser := c.PostForm("username")
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

func DeleteFile(c *gin.Context) {

}
