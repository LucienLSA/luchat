#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "common.h"
#include "chatwidget.h"
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    // 聊天窗口实例
    ChatWidget          *m_pChatWidget;

    // 配置与状态
    QString m_strWsUrl;  // WebSocket服务器地址
    QSettings m_Settings;  // 配置存储（服务器地址、端口等

    // 网络请求管理器
    QNetworkAccessManager  *m_pAccessManager;
};
#endif // MAINWINDOW_H
