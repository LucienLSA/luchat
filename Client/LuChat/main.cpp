#include "mainwindow.h"

#include <QApplication>
#include "common.h"
#include "settingdlg.h"
#include "logindlg.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化应用程序目录
    APPLICATION_DIR = QApplication::applicationDirPath();
    qDebug() << "应用目录" << APPLICATION_DIR;

    // 时间格式化函数
    qDebug() << "当前时间" << FormatTime();

    // 设置组织名和应用名
    QCoreApplication::setOrganizationName("private");
    QCoreApplication::setApplicationName("LuClient");

    // 如果配置信息为空，触发设置对话框
    QSettings settings;
    QString ip = settings.value(CURRENT_SERVER_HOST).toString();
    QString port = settings.value(WEBSOCKET_SERVER_PORT).toString();
    if (ip.isEmpty() || port.isEmpty()) {
        SettingDlg settingdlg;
        if (settingdlg.exec() != QDialog::Accepted) {
            return 0; // 用户取消设置，退出程序
        }
    }

    qDebug() << "ip:" << ip;
    qDebug() << "port:" << port;


    // 登录对话框
    LoginDlg *loginDialog = new LoginDlg();
    int nRet = loginDialog->exec();
    delete loginDialog;

    if (nRet == QDialog::Accepted) {
        // 登录成功，创建并显示主窗口
        MainWindow *w = new MainWindow();
        // 设置主窗口标题
        w->setWindowTitle(g_stUserInfo.strUserPhone);
        w->show();
        return a.exec();
    } else {
        // 取消登录，退出
        exit(-1);
    }
}
