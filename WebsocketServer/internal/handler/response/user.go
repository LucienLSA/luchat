package response

// 用户登录返回
type UserLoginResp struct {
	UserID    int    `json:"userid" form:"userid"`       // 用户id
	UserPhone string `json:"userphone" form:"userphone"` // 登录人手机号
}

// 用户注册
type UserRegisterResp struct {
	UserID    int    `json:"userid" form:"userid"`       // 用户id
	UserPhone string `json:"userphone" form:"userphone"` // 注册手机号
}
