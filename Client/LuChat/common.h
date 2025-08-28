#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QWebSocket>
#include <QSettings>

// 应用版本（用于关于界面或日志）
const QString APPLICATION_VERSION = "1.1.0";


// 配置文件中的键（用于QSettings存储服务器地址、用户信息等）
const QString CURRENT_SERVER_HOST = "CURRENT_SERVER_HOST";       // 当前服务器IP
const QString WEBSOCKET_SERVER_PORT = "WEBSOCKET_SERVER_PORT";   // WebSocket端口
const QString WEBSOCKET_USER_PHONE = "WEBSOCKET_USER_PHONE";       // 用户手机号
const QString WEBSOCKET_USER_ID = "WEBSOCKET_USER_ID";           // 用户ID
const QString WEBSOCKET_USER_PWD = "WEBSOCKET_USER_PWD"; // 用户密码
const QString WEBSOCKET_REMBER_PWD = "WEBSOCKET_REMBER_PWD";   // 是否记住密码

// 消息类型（用于区分不同业务的WebSocket消息）
const QString MSG_TYPE_CHAT = "chat";         // 普通聊天消息
const QString MSG_TYPE_ONLINE = "online";     // 在线用户列表
const QString MSG_TYPE_FILE = "file";         // 文件传输消息
const QString MSG_TYPE_LOGIN = "login";       // 登录状态消息

// 应用路径（全局可访问的程序目录）
extern QString APPLICATION_DIR;


// -------------------------- 数据结构 --------------------------
/**
 * @brief 用户信息结构体（存储登录用户或在线用户的信息）
 */
typedef struct _UserInfo {
    QString strUserPhone;  // 用户名（显示用）
    QString strUserId;    // 用户ID（唯一标识，用于私聊）
    QString strPwd;       // 密码（登录时使用）
    QString strLoginTime; // 登录时间（用于显示在线状态）
    QString strEmail;     // 邮箱（预留）
} UserInfo, *PUserInfo;


/**
 * @brief 消息信息结构体（存储聊天消息的完整内容）
 */
typedef struct _MsgInfo {
//    QString strType;      // 消息类型（MSG_TYPE_CHAT/MSG_TYPE_FILE等）
//    QString strFromUserId;// 发送者ID
    QString strUserPhone;  // 发送者手机号
    QString strUserId;  // 用户ID
    QString strContent;   // 消息内容（文本或文件链接）
    QString strTime;      // 发送时间（格式：yyyy-MM-dd HH:mm:ss）
//    QString strFileSize;  // 文件大小（仅文件消息有值）
    QString fileLink;
    QString strEmail;    // 发送者邮箱（预留）
} MsgInfo, *PMsgInfo;

enum HttpRequest {
    REQUEST_LOGIN, // 登录请求
    REQUEST_REGISTER
};

// -------------------------- 全局变量 --------------------------
extern UserInfo g_stUserInfo;       // 当前登录用户信息
extern QWebSocket g_WebSocket;      // 全局WebSocket对象（用于全项目通信）
extern QSettings g_Settings;        // 全局配置对象（用于读写配置）

// -------------------------- 工具函数 --------------------------
/**
 * @brief 重启应用程序
 */
void RestartApp();

/**
 * @brief 格式化时间为字符串（统一时间格式）
 * @return 格式化后的时间（如"2024-06-01 12:30:45"）
 */
QString FormatTime();


// WebSocket错误信息
const QString WEBSOCKET_ERROR_STRINGS[24] = {
    "未知错误",
    "连接被拒绝（或超时）",
    "远程主机关闭了连接",
    "未找到主机地址",
    "应用程序缺乏必要权限",
    "本地系统资源不足",
    "操作超时",
    "数据报超过系统限制",
    "网络错误（如网线断开）",
    "地址已被占用",
    "绑定的地址不属于本机",
    "操作系统不支持该操作",
    "代理需要认证",
    "SSL/TLS握手失败",
    "操作仍在后台进行",
    "无法连接代理服务器",
    "与代理服务器的连接意外关闭",
    "与代理服务器的连接超时",
    "未找到代理地址",
    "与代理服务器的协商失败",
    "套接字状态不允许操作",
    "SSL库内部错误",
    "SSL数据无效",
    "临时错误（如非阻塞模式下的阻塞操作）"
};

const QHash<QAbstractSocket::SocketError, QString> WEBSOCKET_ERROR_MAP = {
    {QAbstractSocket::UnknownSocketError, "未知错误"},
    {QAbstractSocket::ConnectionRefusedError, "连接被拒绝（或超时）"},
    {QAbstractSocket::RemoteHostClosedError, "远程主机关闭了连接"},
    {QAbstractSocket::HostNotFoundError, "未找到主机地址"},
    {QAbstractSocket::SocketAccessError, "应用程序缺乏必要权限"},
    {QAbstractSocket::SocketResourceError, "本地系统资源不足"},
    {QAbstractSocket::SocketTimeoutError, "操作超时"},
    {QAbstractSocket::DatagramTooLargeError, "数据报超过系统限制"},
    {QAbstractSocket::NetworkError, "网络错误（如网线断开）"},
    {QAbstractSocket::AddressInUseError, "地址已被占用"},
    {QAbstractSocket::SocketAddressNotAvailableError, "绑定的地址不属于本机"},
    {QAbstractSocket::UnsupportedSocketOperationError, "操作系统不支持该操作"},
    {QAbstractSocket::ProxyAuthenticationRequiredError, "代理需要认证"},
    {QAbstractSocket::SslHandshakeFailedError, "SSL/TLS握手失败"},
    {QAbstractSocket::UnfinishedSocketOperationError, "操作仍在后台进行"},
    {QAbstractSocket::ProxyConnectionRefusedError, "无法连接代理服务器"},
    {QAbstractSocket::ProxyConnectionClosedError, "与代理服务器的连接意外关闭"},
    {QAbstractSocket::ProxyConnectionTimeoutError, "与代理服务器的连接超时"},
    {QAbstractSocket::ProxyNotFoundError, "未找到代理地址"},
    {QAbstractSocket::ProxyProtocolError, "与代理服务器的协商失败"},
    {QAbstractSocket::OperationError, "套接字状态不允许操作"},
    {QAbstractSocket::SslInternalError, "SSL库内部错误"},
    {QAbstractSocket::SslInvalidUserDataError, "SSL数据无效"},
    {QAbstractSocket::TemporaryError, "临时错误（如非阻塞模式下的阻塞操作）"}
};

#endif // COMMON_H
