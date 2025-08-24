package response

type ResCode int64

const (
	CodeSuccess ResCode = 200

	CodeInvalidPassword = 1000 + iota
	CodeServerBusy
	CodeInvalidParam

	CodeLoginFailed = 2000 + iota
	CodeTokenInvalid
	CodeLimitLogin
	CodeLoginFailedTooMany

	CodeUserAlreadyLogin
	CodeUserExist
	CodeUserNotExist

	CodeUploadFile = 3000 + iota
	CodeFileTooLarge
	CodeInvalidFileType

	CodeEmailNotExist = 4000 + iota
	CodeEmailExist
	CodeEmailFormat
	CodeInvalidEmailCode
)

var codeMsgMap = map[ResCode]string{
	CodeSuccess:            "success",
	CodeInvalidParam:       "请求参数错误",
	CodeUserExist:          "用户名已存在",
	CodeUserNotExist:       "用户名不存在",
	CodeInvalidPassword:    "用户名或密码错误",
	CodeServerBusy:         "服务出错",
	CodeLoginFailed:        "登录失败",
	CodeTokenInvalid:       "无效的Token",
	CodeLimitLogin:         "已在另一台设备登录",
	CodeUploadFile:         "上传文件失败",
	CodeFileTooLarge:       "文件大小超过限制",
	CodeInvalidFileType:    "文件类型不支持",
	CodeEmailNotExist:      "邮箱不存在",
	CodeEmailExist:         "邮箱已存在",
	CodeEmailFormat:        "邮箱格式有误",
	CodeInvalidEmailCode:   "邮箱验证码无效",
	CodeUserAlreadyLogin:   "用户已登录",
	CodeLoginFailedTooMany: "登录失败次数过多，请输入验证码",
}

func (c ResCode) GetMsg() string {
	msg, ok := codeMsgMap[c]
	if !ok {
		msg = codeMsgMap[CodeServerBusy]
	}
	return msg
}
