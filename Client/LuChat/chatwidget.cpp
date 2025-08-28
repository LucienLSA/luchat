#include "chatwidget.h"
#include "ui_chatwidget.h"
#include <QStandardPaths>
#include <QDateTime>

ChatWidget::ChatWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWidget),
    m_pTextEdit(nullptr),
    m_bIsMainWindow(true),
    m_bCtrlPressed(false)
{
    ui->setupUi(this);
    // 1. 初始化消息输入框
    ui->inputTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);  // 按窗口宽度自动换行
    // 2. 初始化群聊消息显示区域（默认标签页）
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setReadOnly(true);  // 消息区域只读
    m_pTextEdit->setLineWrapMode(QTextEdit::WidgetWidth); // 自动换行


    // 设置大小策略（拉伸比例）
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHorizontalStretch(4);
    policy.setVerticalStretch(3);
    m_pTextEdit->setSizePolicy(policy);

    // 3. 初始化标签页（添加公共聊天窗口）
    // 清除默认标签页
    int nCount = ui->showMsgTabWidget->count();
    for (int i = 0; i < nCount; ++i) {
        ui->showMsgTabWidget->removeTab(0);
    }
//    ui->showMsgTabWidget->clear();
    // 初始化标签页（添加公共聊天窗口）
    ui->showMsgTabWidget->addTab(m_pTextEdit, "聊天窗口");
    ui->showMsgTabWidget->setTabsClosable(true);  // 标签页可关闭（除了公共聊天窗口）
    // 移除群聊标签的关闭按钮
    ui->showMsgTabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->showMsgTabWidget->setSizePolicy(policy);

    // 4. 初始化在线用户列表（QTableWidget）
   ui->onlineUsersTableWidget->setColumnCount(1);  // 只显示用户名
   QTableWidgetItem *item = new QTableWidgetItem("在线用户");
   ui->onlineUsersTableWidget->setHorizontalHeaderItem(0, item);
   ui->onlineUsersTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁止编辑
   ui->onlineUsersTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 列宽自适应

    // 5. 配置分割器（不允许折叠子部件）
    ui->verticalSplitter->setChildrenCollapsible(false);
    ui->horizontalSplitter->setChildrenCollapsible(false);

//     6. 禁用上传按钮（默认未选择文件时不可用）
    ui->uploadFilePushButton->setDisabled(false);

    // 7. 设置消息HTML模板设置占位符
    m_strContentTemplateWithoutLink =
        "<p><strong>%1</strong>：<br>&nbsp;&nbsp;%2&nbsp;&nbsp;<span style='color:gray'>(%3)</span></p>";
    m_strContentTemplateWithLink =
        "<p><strong>%1</strong>：<br>&nbsp;&nbsp;%2&nbsp;&nbsp;<a href='%3'>[文件]</a>&nbsp;&nbsp;<span style='color:gray'>(%4)</span></p>";

    // WebSocket消息接收信号（收到文本消息时触发）
    connect(&g_WebSocket, &QWebSocket::textMessageReceived, this,
            &ChatWidget::OnWebSocketMsgReceived);


//    // 手动连接双击在线用户（发起私聊）
//    connect(ui->onlineUsersTableWidget, &QTableWidget::itemDoubleClicked, this,
//            &ChatWidget::OnItemDoubleClicked);

//    // 手动连接关闭聊天标签页
//    connect(ui->showMsgTabWidget, &QTabWidget::tabCloseRequested, this,
//            &ChatWidget::OnTabCloseRequested);

//    // 手动连接切换聊天标签页
//    connect(ui->showMsgTabWidget, &QTabWidget::currentChanged, this,
//            &ChatWidget::OnCurrentChanged);

    // 注意：`on_uploadFilePushButton_clicked` 和 `on_sendMsgPushButton_clicked`
    // 使用了 Qt 自动连接命名约定，无需手动 connect，避免触发两次
}

ChatWidget::~ChatWidget()
{
    delete m_pTextEdit;
    delete ui;
}

void ChatWidget::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Control) {
        m_bCtrlPressed = true;
    }

    if (m_bCtrlPressed && (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)) {
        on_sendMsgPushButton_clicked();
    }

    if (m_bCtrlPressed && e->key() == Qt::Key_E) {
        SettingDlg *dlg = SettingDlg::GetInstance();
        dlg->exec();
    }
}

void ChatWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control) {
        m_bCtrlPressed = false;
    }
}

// 启用/禁用发送和上传按钮
void ChatWidget::SetSendBtnEnabled(bool enabled)
{
    ui->sendMsgPushButton->setEnabled(enabled);
    ui->uploadFilePushButton->setShortcutEnabled(enabled);
}



// 发送消息按钮
void ChatWidget::on_sendMsgPushButton_clicked()
{
    // 获取消息输入框的文本
    QString msg = ui->inputTextEdit->toPlainText().trimmed();
    if (msg.isEmpty()) {
        return;
    }
    // 当前聊天对话框的索引
    int curTabIndex = ui->showMsgTabWidget->currentIndex();
    //定义消息中的message
    QJsonObject jsonObj;
    jsonObj["userphone"]=g_stUserInfo.strUserPhone;
    jsonObj["userid"] =g_stUserInfo.strUserId;
    jsonObj["message"] = msg;
    jsonObj["filelink"] = m_strFileLink; // 文件链接
    jsonObj["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 定义消息json格式
    QJsonObject jsonMsg;
    if (curTabIndex == 0 ) {
        // 公共消息
        jsonMsg["message"] = jsonObj;
    } else {
        // 私聊消息 获取当前私聊用户ID列表
        jsonMsg[m_vecUserIds[curTabIndex-1]] = jsonObj;
    }

    // 发送WebSocket消息
    g_WebSocket.sendTextMessage(QJsonDocument(jsonMsg).toJson(QJsonDocument::Compact));

    // 更新界面UI
    ui->inputTextEdit->clear(); // 清空输入对话框
    // 公共消息/群聊
    if (curTabIndex == 0) {
        if (!m_strFileLink.isEmpty()) {
            m_jStringMessages["message"] += m_strContentTemplateWithLink
                    .arg(g_stUserInfo.strUserPhone).arg(msg).arg(m_strFileLink).arg(jsonObj["time"].toString());
        } else {
            m_jStringMessages["message"] += m_strContentTemplateWithoutLink
                    .arg(g_stUserInfo.strUserPhone).arg(msg).arg(jsonObj["time"].toString());
        }
        m_pTextEdit->setHtml(m_jStringMessages["message"]);
        // 清空文件链接
        m_strFileLink.clear();
    } else {
        QString targetUserID = m_vecUserIds[curTabIndex-1];
        if (!m_strFileLink.isEmpty()) {
            m_jStringMessages[targetUserID] += m_strContentTemplateWithLink
                    .arg(g_stUserInfo.strUserPhone).arg(msg).arg(m_strFileLink).arg(jsonObj["time"].toString());
        } else {
            m_jStringMessages[targetUserID] += m_strContentTemplateWithoutLink
                    .arg(g_stUserInfo.strUserPhone).arg(msg).arg(jsonObj["time"].toString());
        }
        QTextEdit *tabEdit = qobject_cast<QTextEdit*>(ui->showMsgTabWidget->widget(curTabIndex));
        if (tabEdit) {
            tabEdit->setHtml(m_jStringMessages[targetUserID]);
        }
        // 清空文件链接
        m_strFileLink.clear();
    }
}

// 点击上传文件
void ChatWidget::on_uploadFilePushButton_clicked()
{
    const QString startDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString filter =
            "所有文件 (*.*);;"
            "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp);;"
            "文档 (*.txt *.pdf *.doc *.docx *.xls *.xlsx);;"
            "压缩包 (*.zip *.rar *.7z)";
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", startDir, filter);
    if (!filePath.isEmpty()) {
        // 发出上传文件信号
        emit uploadFile(filePath);
    }
}

// 接收WebSocket消息
void ChatWidget::OnWebSocketMsgReceived(const QString &msg)
{
    QJsonParseError err;
    // 解析消息为Json
    QJsonDocument jsonDoc = QJsonDocument::fromJson(msg.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "解析消息失败:" << err.error;
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.contains("message")) {
        // 公共消息
        QJsonObject msgObj = jsonObj["message"].toObject();
        QString senderId = msgObj["userid"].toString();
        // 忽略自己发的消息
        if (senderId == g_stUserInfo.strUserId)
        {
            return;
        }

        MsgInfo msgInfo ;
        msgInfo.strUserId = senderId;
        msgInfo.strUserPhone = msgObj["userphone"].toString();
        msgInfo.strContent = msgObj["message"].toString();
        msgInfo.strTime = msgObj["time"].toString();
        msgInfo.fileLink = msgObj["filelink"].toString();

        // 更新公共聊天窗口
        if (msgInfo.fileLink.isEmpty()) {
            m_jStringMessages["message"] += m_strContentTemplateWithoutLink
                    .arg(msgInfo.strUserPhone).arg(msgInfo.strContent).arg(msgInfo.strTime);
        } else {
            m_jStringMessages["message"] += m_strContentTemplateWithLink
                            .arg(msgInfo.strUserPhone).arg(msgInfo.strContent).arg(msgInfo.fileLink).arg(msgInfo.strTime);
        }

        // 渲染消息html
        m_pTextEdit->setHtml(m_jStringMessages["message"]);
        // 发送提醒消息
        emit newMessageArrived();
        // 更新在线用户
    } else if (jsonObj.contains("online")) {
        QJsonObject onlineObj = jsonObj["online"].toObject();
        QString userId = onlineObj["userid"].toString();
        if (userId == g_stUserInfo.strUserId) {
            // return;
        }

        UserInfo user;
        user.strUserId = userId;
        user.strUserPhone = onlineObj["userphone"].toString();
        // 避免重复添加
        bool exists = false;
        for (auto &u : m_vecOnlineUsers) {
            if (u.strUserId == userId) {
                exists = true;
                break;
            }
        }
        // 不存在则添加
        if (!exists) {
            m_vecOnlineUsers.push_back(user);
        }

        // 更新在线用户列表
        ui->onlineUsersTableWidget->setRowCount(m_vecOnlineUsers.size());
        for (int i= 0; i < m_vecOnlineUsers.size(); ++i) {
            ui->onlineUsersTableWidget->setItem(i,0,new QTableWidgetItem(m_vecOnlineUsers[i].strUserPhone));
        }
    } else if (jsonObj.contains(g_stUserInfo.strUserId)) {
        // 私聊消息
        QJsonObject msgObj = jsonObj[g_stUserInfo.strUserId].toObject();
        QString senderId = msgObj["userid"].toString();
        QString senderPhone = msgObj["userphone"].toString();

        // 查找或者创建私聊标签页
        int tabIndex = -1;
        for (int i = 0; i < m_vecUserIds.size();++i) {
            if (m_vecUserIds[i] == senderId) {
                tabIndex = i+1;
                break;
            }
        }
        if (tabIndex == -1 ) {
            // 新建
            QTextEdit *newEdit = new QTextEdit();
            newEdit->setReadOnly(true);
            tabIndex = ui->showMsgTabWidget->addTab(newEdit, senderPhone);
            m_vecUserIds.push_back(senderId);
        }

        // 更新私聊窗口内容
        QString time = msgObj["time"].toString();
        QString content = msgObj["message"].toString();
        QString fileLink = msgObj["filelink"].toString();
        if (fileLink.isEmpty()) {
            m_jStringMessages[senderId] += m_strContentTemplateWithoutLink
                .arg(senderPhone).arg(content).arg(time);
        } else {
            m_jStringMessages[senderId] += m_strContentTemplateWithLink
                .arg(senderPhone).arg(content).arg(fileLink).arg(time);
        }
        QTextEdit *tabEdit = qobject_cast<QTextEdit*>(ui->showMsgTabWidget->widget(tabIndex));
        if (tabEdit) tabEdit->setHtml(m_jStringMessages[senderId]);

        emit newMessageArrived(); // 提醒新消息
    }

}

// 双击在线用户发起私聊
void ChatWidget::OnItemDoubleClicked(QTableWidgetItem *item)
{
    // 获取点击的位置
    int row = item->row();
    if (row < 0 || row >= m_vecOnlineUsers.size()) return;
    UserInfo targetUser = m_vecOnlineUsers[row];
    // 检查是否已经存在私聊窗口
    for (int i = 0; i < m_vecUserIds.size();++i) {
        if (m_vecUserIds[i] == targetUser.strUserId) {
            // 切换到该用户的私聊标签页
            ui->showMsgTabWidget->setCurrentIndex(i+1);
            return ;
        }
    }
    // 新建私聊标签页
    QTextEdit *newEdit = new QTextEdit();
    newEdit->setReadOnly(true);
    int tabIndex = ui->showMsgTabWidget->addTab(newEdit, targetUser.strUserPhone);
    m_vecUserIds.push_back(targetUser.strUserId);
    ui->showMsgTabWidget->setCurrentIndex(tabIndex);
}

// 关闭标签页
void ChatWidget::OnTabCloseRequested(int index)
{
    // 公共聊天标签页禁止关闭
     if (index == 0 ) {
         return;
     }
     // 删除标签页索引
     ui->showMsgTabWidget->removeTab(index);
     // 删除私聊用户id
     m_vecUserIds.removeAt(index-1);
}

// 切换标签页
void ChatWidget::OnCurrentChanged(int index)
{

}
