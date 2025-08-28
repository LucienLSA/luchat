#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      m_pChatWidget(nullptr),
      m_pAccessManager(nullptr)
{
    ui->setupUi(this);

    // 初始化聊天对话框
    m_pChatWidget = new ChatWidget();
    setCentralWidget(m_pChatWidget);

    // 加载服务器地址并连接WebSocket
    QString ip = m_Settings.value(CURRENT_SERVER_HOST).toString();
    QString port = m_Settings.value(WEBSOCKET_SERVER_PORT).toString();
    m_strWsUrl = QString("ws://%1:%2/ws").arg(ip).arg(port);
    g_WebSocket.open(QUrl(m_strWsUrl));


    // 初始化文件网络请求管理器，网络请求（文件上传）完成信号
    m_pAccessManager = new QNetworkAccessManager(this);
    connect(m_pAccessManager, &QNetworkAccessManager::finished,this,
            &MainWindow::replyFinished);

    // 绑定WebSocket信号槽
    // WeSocket连接信号到来，调用函数
    connect(&g_WebSocket, &QWebSocket::connected, this, &MainWindow::OnWebSocketConnected);
    // 断开WeSocket连接，调用函数
    connect(&g_WebSocket, &QWebSocket::disconnected, this, &MainWindow::OnWebSocketDisconnected);
//    connect(&g_WebSocket, &QWebSocket::errorOccurred, this, &MainWindow::OnWebSocketError);
    // WeSocket出错，调用错误函数
    connect(&g_WebSocket, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            this, &MainWindow::OnWebSocketError);

    // 绑定聊天界面信号 实现聊天窗口与WebSocket当前进行信号传递
    // 新消息到达
    connect(m_pChatWidget, &ChatWidget::newMessageArrived, this, &MainWindow::OnNewMessageArrived);
    // 文件上传请求
    connect(m_pChatWidget, &ChatWidget::uploadFile, this, &MainWindow::OnUploadFile);
}

MainWindow::~MainWindow()
{
    if (g_WebSocket.isValid()) {
        g_WebSocket.close();
    }
    delete m_pChatWidget;
    delete m_pAccessManager;
    delete m_pProgressDlg;
    delete ui;
}

// WebSocket连接成功处理函数
void MainWindow::OnWebSocketConnected()
{
    qDebug() << "WebSocket发送成功";
    QJsonObject jsonObj;
    jsonObj["userphone"] = g_stUserInfo.strUserPhone;
    jsonObj["userid"] = g_stUserInfo.strUserId;
    QJsonObject onlineObj;
    onlineObj["online"] = jsonObj;
    // 发送当前在线用户json消息
    g_WebSocket.sendTextMessage(QJsonDocument(onlineObj).toJson(QJsonDocument::Compact));
    // 启用发送按钮
    m_pChatWidget->SetSendBtnEnabled(true);
}

void MainWindow::OnWebSocketDisconnected()
{
    qDebug() << "WebSocket断开，尝试重连...";
    // 禁用发送和上传按钮
    m_pChatWidget->SetSendBtnEnabled(false);
    // 重连
    g_WebSocket.open(QUrl(m_strWsUrl));
}

// WebSocket错误
void MainWindow::OnWebSocketError(QAbstractSocket::SocketError err) {
//    qDebug() << "WebSocket错误：" << WEBSOCKET_ERROR_STRINGS[err + 1];
    qDebug() << "错误代码:" << err;
    qDebug() << "错误描述:" << g_WebSocket.errorString(); // 这是最重要的信息
//    映射
    int index = static_cast<int>(err);
    if (index >= 0 && index < 24) {
        qDebug() << "映射错误:" << WEBSOCKET_ERROR_STRINGS[index];
    }
}

void MainWindow::OnNewMessageArrived()
{
    QApplication::alert(this); // 窗口闪烁提醒
}

// 点击文件上传后，调用
void MainWindow::OnUploadFile(QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this,"错误","文件打开失败");
        return;
    }
    // 读取文件内容
    QByteArray fileData = file.readAll();
    file.close();

    // 构建上传URL
    // 提取文件名（兼容各平台路径分隔符）
    QString fileName = QFileInfo(filePath).fileName();
    QString ip = m_Settings.value(CURRENT_SERVER_HOST).toString();
    QString port = m_Settings.value(WEBSOCKET_SERVER_PORT).toString();
    QUrl url(QString("http://%1:%2/upload?filename=%3").arg(ip).arg(port).arg(fileName));

    // 设置请求url
    QNetworkRequest req(url);
    // 设置请求头
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    // 发送上传请求，并返回响应给reply
    QNetworkReply *reply = m_pAccessManager->post(req,fileData);
    // 绑定上传信号，错误发生处理函数，文件上传进度条
    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this,&MainWindow::upLoadError);
    connect(reply, &QNetworkReply::uploadProgress, this,&MainWindow::OnUploadProgress);

    // 显示进度对话框
    if (!m_pProgressDlg) {
        m_pProgressDlg = new QProgressDialog("正在上传...","取消", 0, fileData.size(), this);
        m_pProgressDlg->setWindowTitle("上传文件");
        m_pProgressDlg->setAutoClose(false);
        m_pProgressDlg->setAutoReset(false);
    }
    m_pProgressDlg->show();
    // 允许用户取消：仅中止当前 reply，最终删除由 finished 统一处理
    connect(m_pProgressDlg, &QProgressDialog::canceled, this, [this, reply]() {
        reply->abort();
        if (m_pProgressDlg) {
            m_pProgressDlg->hide();
        }
    });
}

// 文件请求信号完成，接收响应
void MainWindow::replyFinished(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
        QMessageBox::information(this, "成功","文件上传成功");
    } else {
        QMessageBox::warning(this, "失败","文件上传失败");
    }
    // 上传完成，隐藏消息对话框
    if (m_pProgressDlg) {
        m_pProgressDlg->hide();
    }
    reply->deleteLater();
}

// 上传错误
void MainWindow::upLoadError(QNetworkReply::NetworkError err)
{
    qDebug() << "上传错误：" << err;
    if (m_pProgressDlg) m_pProgressDlg->hide();
}

// 上传进度条设置接收数据和总文件大小
void MainWindow::OnUploadProgress(qint64 recved, qint64 total)
{
    if (m_pProgressDlg) {
        m_pProgressDlg->setMaximum(total);
        m_pProgressDlg->setValue(recved);
    }
}

