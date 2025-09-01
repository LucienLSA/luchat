#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include "common.h"
#include <QTableWidgetItem>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFileDialog>
#include <QKeyEvent>
#include <settingdlg.h>

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();


    // 设置发送按钮状态（连接成功/失败时调用）
    void SetSendBtnEnabled(bool enabled);
    
    // 添加当前用户到在线列表
    void AddCurrentUserToOnlineList();

signals:
    void newMessageArrived(); // 新消息提醒
    void uploadFile(QString filePath); // 上传文件信号


private slots:
    // 发送消息
    void on_sendMsgPushButton_clicked();
    // 上传文件
    void on_uploadFilePushButton_clicked();
    // 接收WebSocket消息
    void OnWebSocketMsgReceived(const QString &msg);
    // 双击在线用户发起私聊
    void on_onlineUsersTableWidget_itemDoubleClicked(QTableWidgetItem *item);
     // 关闭聊天标签页
    void on_showMsgTabWidget_tabCloseRequested(int index);
    // 切换标签页
    void on_showMsgTabWidget_currentChanged(int index);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

private:
    Ui::ChatWidget *ui;
    // 消息显示区域
    QTextEdit *m_pTextEdit;
    // 标记是否为主窗口（群聊窗口）
    bool m_bIsMainWindow;
    // Ctrl键状态
    bool m_bCtrlPressed;
    // 私聊用户ID列表
    QVector<QString> m_vecUserIds;
    // 按用户ID存储聊天记录
    QMap<QString, QString> m_jStringMessages;
    // 消息列表
    QVector<MsgInfo> m_vecMsgInfos;
    // 在线用户列表
    QVector<UserInfo> m_vecOnlineUsers;
    // 最近上传的文件链接
    QString m_strFileLink;

    // 消息模板（HTML格式，方便格式化显示）
    QString m_strContentTemplateWithLink;  // 带文件链接的消息模板
    QString m_strContentTemplateWithoutLink;  // 无链接的消息模板
};

#endif // CHATWIDGET_H
