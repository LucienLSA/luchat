#include "common.h"
#include <QApplication>
#include <QDateTime>
#include <QProcess>
#include <QDir>

// -------------------------- 全局变量初始化 --------------------------
QString APPLICATION_DIR = "";  // 应用程序目录（在main函数中初始化）
// 初始化全局用户信息（空值）
UserInfo g_stUserInfo;

// 初始化全局WebSocket实例
QWebSocket g_WebSocket;

//// 配置文件存储路径：Windows在注册表，Linux在~/.config，Mac在~/Library/Preferences
//QSettings g_Settings("LuChat", "LuChat");


// -------------------------- 工具函数实现 --------------------------
void RestartApp() {
    // 获取当前应用程序路径
    QString appPath = QApplication::applicationFilePath();
    // 启动新进程并退出当前进程
    QProcess::startDetached(appPath, QStringList());
    QApplication::exit();
}

QString FormatTime(){
    // 获取当前时间并格式化为"yyyy-MM-dd HH:mm:ss"
    return  QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

