package response

import "errors"

var (
	ErrorUserExist          = errors.New("用户已存在")
	ErrorUserNotExist       = errors.New("用户不存在")
	ErrorEmailNotExit       = errors.New("邮箱不存在")
	ErrorEmailExist         = errors.New("邮箱已存在")
	ErrorInvalid            = errors.New("用户名或者密码错误")
	ErrorUserAlreadyLogin   = errors.New("用户已登录")
	ErrorLoginFailedTooMany = errors.New("登录失败次数过多")
	ErrorServerBusy         = errors.New("服务器繁忙")

	ErrorNeedLogin  = errors.New("需要用户登录")
	ErrorLimitLogin = errors.New("登录已失效")
	ErrorInvalidID  = errors.New("无效的ID")
	// Error
)
