package request

type LoginReq struct {
	UserName      string `json:"username" binding:"required"`
	Password      string `json:"password" binding:"required"`
	ClientVersion string `json:"client_version"`
}

type RegisterReq struct {
	UserName string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}
