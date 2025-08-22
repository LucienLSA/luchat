package global

// 客户端版本相关
type NewClinetItem struct {
	NewClient     string
	UserId        int
	VersionNumber int
	FileName      string
	Md5Value      string
}

type ClientItem struct {
	Flag          bool
	FileName      string
	Md5Value      string
	VersionNumber int
}

type HttpResponse2 struct {
	Success   bool
	Msg       string
	Id        int
	Username  string
	NewClient ClientItem
}
