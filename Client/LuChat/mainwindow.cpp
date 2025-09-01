#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      m_pChatWidget(nullptr),
      m_pAccessManager(nullptr),
      m_pProgressDlg(nullptr)
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
    // 添加当前用户到在线列表
    m_pChatWidget->AddCurrentUserToOnlineList();
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


// 检查网络是否可用
bool MainWindow::isNetworkAvailable() const
{
    QNetworkConfigurationManager manager;
    return manager.isOnline();
}

void MainWindow::showUploadProgressDialog(const QString &fileName, qint64 fileSize, QNetworkReply *reply)
{
    // 创建或重置进度对话框
        if (!m_pProgressDlg) {
            m_pProgressDlg = new QProgressDialog(this);
            m_pProgressDlg->setWindowTitle("上传文件"); // 设置标题
            m_pProgressDlg->setLabelText("正在上传..."); // 设置标签文本
            m_pProgressDlg->setCancelButtonText("取消"); // 取消按钮文本
            m_pProgressDlg->setMinimumDuration(0); // 进度条最小值
            m_pProgressDlg->setAutoClose(true);
            m_pProgressDlg->setAutoReset(true);
        }

        m_pProgressDlg->setLabelText(QString("正在上传: %1").arg(fileName));
        m_pProgressDlg->setMaximum(fileSize); // 最大值文件大小
        m_pProgressDlg->setValue(0);
        m_pProgressDlg->show();

        // 连接取消信号，当点击取消按钮，中断上传
        connect(m_pProgressDlg, &QProgressDialog::canceled, this, [this, reply]() {
            if (reply && reply->isRunning()) {
                reply->abort();
            }
            if (m_pProgressDlg) {
                m_pProgressDlg->hide();
            }
        });

        // 连接finished信号来自动关闭对话框
        connect(reply, &QNetworkReply::finished, this, [this]() {
            if (m_pProgressDlg && m_pProgressDlg->isVisible()) {
                m_pProgressDlg->hide();
            }
        });
}

// 点击文件上传后，调用
void MainWindow::OnUploadFile(const QString &filePath)
{
    // 1. 验证文件路径
        if (filePath.isEmpty()) {
            QMessageBox::warning(this, "错误", "文件路径为空");
            return;
        }
        // 2. 检查文件是否存在和可读
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists() || !fileInfo.isReadable()) {
            QMessageBox::warning(this, "错误", QString("文件不存在或不可读: %1").arg(filePath));
            return;
        }

        // 3. 检查文件大小（避免上传过大文件）
        const qint64 maxFileSize = 100 * 1024 * 1024; // 100MB
        if (fileInfo.size() > maxFileSize) {
            QMessageBox::warning(this, "文件过大",
                QString("文件大小 (%1 MB) 超过限制 (%2 MB)")
                    .arg(fileInfo.size() / (1024.0 * 1024.0), 0, 'f', 1)
                    .arg(maxFileSize / (1024 * 1024)));
            return;
        }

        // 4. 检查网络连接状态
        if (!isNetworkAvailable()) {
            QMessageBox::warning(this, "网络错误", "网络连接不可用");
            return;
        }


        // 5. 构建URL（添加参数验证）
        QString ip = m_Settings.value(CURRENT_SERVER_HOST).toString();
        QString port = m_Settings.value(WEBSOCKET_SERVER_PORT).toString();

        if (ip.isEmpty() || port.isEmpty()) {
            QMessageBox::warning(this, "配置错误", "服务器地址或端口未配置");
            return;
        }

        QUrl url(QString("http://%1:%2/api/upload").arg(ip).arg(port));
        if (!url.isValid()) {
            QMessageBox::warning(this, "错误", "无效的URL地址");
            return;
        }


        // 使用 multipart/form-data 适配服务端 c.FormFile("file")
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        // 文件部分
        QHttpPart filePart;
        // 设置Header
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant(QString("form-data; name=\"file\"; filename=\"%1\"")
                                   .arg(fileInfo.fileName())));

        // 绑定文件数据为设备，避免一次性读入内存
        QFile *upFile = new QFile(filePath);
        if (!upFile->open(QIODevice::ReadOnly)) {
            delete upFile;
            delete multiPart;
            QMessageBox::warning(this, "错误", "文件打开失败");
            return;
        }
        filePart.setBodyDevice(upFile);
        upFile->setParent(multiPart); // 由 multiPart 管理释放
        multiPart->append(filePart);

        // 用户名部分（服务端使用 PostForm("userphone")）
        QHttpPart userPart;
        userPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant("form-data; name=\"userphone\""));
        userPart.setBody(g_stUserInfo.strUserPhone.toUtf8());
        multiPart->append(userPart);

        // 发起请求
        QNetworkRequest req(url);
        req.setRawHeader("Accept", "application/json");
        QNetworkReply *reply = m_pAccessManager->post(req, multiPart);
        multiPart->setParent(reply); // reply 删除时一并释放


        // 绑定上传信号，错误发生处理函数，文件上传进度条
        connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this,&MainWindow::upLoadError);

        connect(reply, &QNetworkReply::uploadProgress, this,&MainWindow::OnUploadProgress);

        // 删除由 replyFinished 统一负责，不在此处重复 deleteLater

        // 10. 显示进度对话框（改进用户体验）
        showUploadProgressDialog(fileInfo.fileName(), fileInfo.size(), reply);
    }

    // 文件请求信号完成，接收响应
    void MainWindow::replyFinished(QNetworkReply *reply)
    {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        qDebug() << "Upload finished, status:" << httpStatus << ", error:" << reply->error()
                 << ", errorString:" << reply->errorString() << ", body:" << body;

        if (reply->error() == QNetworkReply::NoError && httpStatus == 200) {
            QMessageBox::information(this, "成功", "文件上传成功");
        } else {
            QString reason = reply->errorString();
            if (!body.isEmpty()) {
                reason += "\n" + QString::fromUtf8(body);
            }
            QMessageBox::warning(this, "失败", QString("文件上传失败\n%1").arg(reason));
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

