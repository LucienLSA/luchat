#ifndef REGISTRYDLG_H
#define REGISTRYDLG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QRegExpValidator>
#include <QValidator>
#include <QMessageBox>
#include <QSettings>
#include "common.h"

namespace Ui {
class RegistryDlg;
}

class RegistryDlg : public QDialog
{
    Q_OBJECT

public:
    explicit RegistryDlg(QWidget *parent = nullptr);
    ~RegistryDlg();

signals:
    void registerSuccess(const QString &userName);

private slots:

    void on_cancelpushButton_clicked();

    void on_registerReplyFinished(QNetworkReply *reply);  // 注册请求响应处理

    void on_registrypushButton_clicked();

private:
    Ui::RegistryDlg *ui;
    QNetworkAccessManager *m_pRegisterManager; // 注册请求管理器
    bool validateInput(QString &errorMsg);
    bool IsValidPhoneNumber(const QString & phoneNum);
    
    // 获取服务器配置
    bool getServerConfig(QString &ip, QString &port);
};

#endif // REGISTRYDLG_H
