package request

type LoginReq struct {
	UserPhone     string `json:"userphone" binding:"required"`
	Password      string `json:"password" binding:"required"`
	ClientVersion string `json:"client_version"`
}

type RegisterReq struct {
	UserPhone string `json:"userphone" binding:"required"`
	Password  string `json:"password" binding:"required"`
}
