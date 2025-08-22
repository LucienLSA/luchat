package logger

import (
	"os"
	"path/filepath"
	"time"

	"github.com/gin-gonic/gin"
	rotatelogs "github.com/lestrrat/go-file-rotatelogs"
	"github.com/rifflock/lfshook"
	"github.com/sirupsen/logrus"
)

func Init(logPath, logFileName string) {
	// Ensure log directory exists
	if err := os.MkdirAll(logPath, 0o755); err != nil {
		logrus.Fatalf("日志目录创建失败: %v", err)
	}

	baselogPath := filepath.Join(logPath, logFileName)
	writer, err := rotatelogs.New(
		baselogPath+".%Y%m%d%H%M",
		rotatelogs.WithLinkName(baselogPath),
		rotatelogs.WithMaxAge(30*24*time.Hour),
		rotatelogs.WithRotationTime(24*time.Hour),
	)
	if err != nil {
		logrus.Fatalf("日志初始化失败: %v", err)
	}

	// Base logrus config
	logrus.SetOutput(os.Stdout)
	logrus.SetReportCaller(false)
	logrus.SetLevel(logrus.InfoLevel)
	logrus.SetFormatter(&logrus.TextFormatter{FullTimestamp: true})
	lfHook := lfshook.NewHook(lfshook.WriterMap{
		logrus.DebugLevel: writer,
		logrus.InfoLevel:  writer,
		logrus.WarnLevel:  writer,
		logrus.ErrorLevel: writer,
		logrus.FatalLevel: writer,
		logrus.PanicLevel: writer,
	}, &logrus.TextFormatter{})
	logrus.AddHook(lfHook)
}

// GinLogger  Gin日志中间件
func GinLogger() gin.HandlerFunc {
	return func(c *gin.Context) {
		startTime := time.Now()
		path := c.Request.URL.Path
		query := c.Request.URL.RawQuery

		c.Next()

		latency := time.Since(startTime)
		statusCode := c.Writer.Status()
		clientIP := c.ClientIP()
		method := c.Request.Method
		errs := c.Errors.ByType(gin.ErrorTypePrivate).String()

		entry := logrus.WithFields(logrus.Fields{
			"status":  statusCode,
			"method":  method,
			"path":    path,
			"query":   query,
			"ip":      clientIP,
			"latency": latency.String(),
		})
		if errs != "" {
			entry.Error(errs)
		} else {
			entry.Info("request completed")
		}
	}
}
