#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "common.h"
#include "chatwidget.h"
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void OnWebSocketConnected();    // WebSocket连接成功
    void OnWebSocketDisconnected(); // WebSocket断开
    void OnWebSocketError(QAbstractSocket::SocketError err); // 连接错误
    void OnNewMessageArrived();     // 新消息提醒
    void OnUploadFile(QString filePath); // 处理文件上传
    void replyFinished(QNetworkReply *reply); // 上传响应
    void upLoadError(QNetworkReply::NetworkError err); // 上传错误
    void OnUploadProgress(qint64 recved, qint64 total); // 上传进度

private:
    Ui::MainWindow *ui;
    // 聊天窗口实例
    ChatWidget          *m_pChatWidget;

    // 配置与状态
    QString m_strWsUrl;  // WebSocket服务器地址
    QSettings m_Settings;  // 配置存储（服务器地址、端口等

    // 文件上传网络请求管理器
    QNetworkAccessManager  *m_pAccessManager;
    // 上传进度对话框
    QProgressDialog *m_pProgressDlg;
};
#endif // MAINWINDOW_H
