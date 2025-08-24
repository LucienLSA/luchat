#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "registrydlg.h"

namespace Ui {
class LoginDlg;
}

class LoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDlg(QWidget *parent = nullptr);
    ~LoginDlg();

private slots:

    void on_registrypushButton_clicked();

    void on_loginReplyFinished(QNetworkReply *reply);  // 登录请求响应处理

    void on_serverpushButton_clicked();

    void on_loginpushButton_clicked();

    void on_registerSuccess(const QString &userPhone);

private:
    Ui::LoginDlg *ui;
    QNetworkAccessManager *m_pLoginManager;  // 登录请求管理器
    bool m_bCtrlPressed;  // 记录Ctrl键是否按下

    // 从配置读取保存的用户名和密码
    void loadSavedUserInfo();

    // 保存用户名和密码到配置
    void saveUserInfo(const QString &userPhone, const QString &password);

    // 发送登录请求
    void sendLoginRequest(const QString &userPhone, const QString &password);

    // 发送注册请求
    void sendRegisterRequest(const QString &userPhone, const QString &password);
    
//    // 测试网络连接
//    void testNetworkConnection();

    RegistryDlg *m_pRegisterDialog;
};

#endif // LOGINDLG_H
