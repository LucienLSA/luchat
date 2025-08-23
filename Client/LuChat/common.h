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
const QString WEBSOCKET_USER_NAME = "WEBSOCKET_USER_NAME";       // 用户名
const QString WEBSOCKET_USER_ID = "WEBSOCKET_USER_ID";           // 用户ID
const QString AUTO_LOGIN = "AUTO_LOGIN";                         // 自动登录标志

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
    QString strUserName;  // 用户名（显示用）
    QString strUserId;    // 用户ID（唯一标识，用于私聊）
    QString strPwd;       // 密码（登录时使用）
    QString strLoginTime; // 登录时间（用于显示在线状态）
} UserInfo, *PUserInfo;


/**
 * @brief 消息信息结构体（存储聊天消息的完整内容）
 */
typedef struct _MsgInfo {
    QString strType;      // 消息类型（MSG_TYPE_CHAT/MSG_TYPE_FILE等）
    QString strFromUserId;// 发送者ID
    QString strFromName;  // 发送者名称
    QString strToUserId;  // 接收者ID（空表示群聊）
    QString strContent;   // 消息内容（文本或文件链接）
    QString strTime;      // 发送时间（格式：yyyy-MM-dd HH:mm:ss）
    QString strFileSize;  // 文件大小（仅文件消息有值）
} MsgInfo, *PMsgInfo;

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


#endif // COMMON_H
