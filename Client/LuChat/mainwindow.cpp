#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      m_pChatWidget(nullptr)
{
    ui->setupUi(this);
    // 初始化聊天对话框
    m_pChatWidget = new ChatWidget();
    setCentralWidget(m_pChatWidget);

    // WebSocket状态信号连接（连接成功/断开/错误）
    connect(&g_WebSocket, SIGNAL(connected()), this, SLOT(OnWebSocketConnected()));
    connect(&g_WebSocket, SIGNAL(disconnected()), this, SLOT(OnWebSocketDisconnected()));
    connect(&g_WebSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(OnWebSocketError(QAbstractSocket::SocketError)));

    // 初始化网络请求管理器
    m_pAccessManager = new QNetworkAccessManager();
    connect(m_pAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(replyFinished(QNetworkReply *)));


    // 网络请求（文件上传）完成信号
    connect(m_pAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(replyFinished(QNetworkReply *)));

    // ChatWidget（UI层）与MainWindow（管理层）的跨组件信号
    connect(m_pChatWidget, SIGNAL(newMessageArrived()), this,
            SLOT(OnNewMessageArrived())); // 新消息到达提醒
    connect(m_pChatWidget, SIGNAL(uploadFile(QString)), this,
            SLOT(OnUploadFile(QString))); // 文件上传请求
}

MainWindow::~MainWindow()
{
    delete ui;
}

