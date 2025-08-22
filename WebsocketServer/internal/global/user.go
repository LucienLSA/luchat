package global

// 在线用户
type OnlineSt struct {
	Userid   string
	Username string
}

type OnlineUser struct {
	Online OnlineSt
	Addr   string
}
