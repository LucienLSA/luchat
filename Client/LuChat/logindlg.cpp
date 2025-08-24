#include "logindlg.h"
#include "ui_logindlg.h"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include "settingdlg.h"
#include "common.h"
#include "registrydlg.h"

LoginDlg::LoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlg),
    m_pLoginManager(nullptr),
    m_bCtrlPressed(false),
    m_pRegisterDialog(nullptr)
{
    ui->setupUi(this);

    // 移除对话框右上角的问号按钮
    Qt::WindowFlags flags = this->windowFlags();
    setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setWindowTitle("用户登录");

    // 初始化网络请求管理器
    // 当登录请求完成完成后，调用函数
    m_pLoginManager = new QNetworkAccessManager(this);
    connect(m_pLoginManager, &QNetworkAccessManager::finished,
            this,&LoginDlg::on_loginReplyFinished);

    // 初始化注册对话框并关联信号
    // 接收注册对话框发送的注册成功信号，调用函数
    m_pRegisterDialog = new RegistryDlg(this);
    connect(m_pRegisterDialog, &RegistryDlg::registerSuccess,
            this,&LoginDlg::on_registerSuccess);


    // 加载保存的用户信息
    loadSavedUserInfo();
    
    // 手动连接服务器配置按钮的信号槽
    connect(ui->serverpushButton, &QPushButton::clicked, 
            this, &LoginDlg::on_serverpushButton_clicked);
    
    // 手动连接登录按钮的信号槽
    connect(ui->loginpushButton, &QPushButton::clicked,
            this, &LoginDlg::on_loginpushButton_clicked);
    
    // 手动连接注册按钮的信号槽
    connect(ui->registrypushButton, &QPushButton::clicked,
            this, &LoginDlg::on_registrypushButton_clicked);
}

LoginDlg::~LoginDlg()
{
    delete ui;
    // 释放空间
    if (m_pLoginManager) delete m_pLoginManager;
    if (m_pRegisterDialog) delete m_pRegisterDialog;
}


// 点击登录按钮事件
void LoginDlg::on_loginpushButton_clicked()
{
        QString userPhone = ui->phonelineEdit->text().trimmed();
        QString password = ui->passwordlineEdit->text().trimmed();

        // 输入验证
        if(userPhone.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "提示","用户手机号或者密码不能为空");
                    return;
        }
        // 点击一次之后禁用，防止重复提交
        ui->loginpushButton->setEnabled(false);
        ui->loginpushButton->setText("登陆中...");

        // 发送登录网络请求
        qDebug() << "userphone" << userPhone;
        qDebug() << "password" << password;

        sendLoginRequest(userPhone,password);
}

// 注册成功回调函数
void LoginDlg::on_registerSuccess(const QString &userPhone)
{
     // 注册成功自动将用户手机号填入登录框
    ui->phonelineEdit->setText(userPhone);
    // 清空当前密码框
    ui->passwordlineEdit->clear();
    // 指针聚焦
    ui->passwordlineEdit->setFocus();
}


// 发送登录请求
void LoginDlg::sendLoginRequest(const QString &userPhone, const QString &password)
{
    qDebug() << "开始发送登录请求...";
    
    // 从配置获取服务器信息
    QSettings settings;
    QString ip = settings.value(CURRENT_SERVER_HOST).toString();
    QString port = settings.value(WEBSOCKET_SERVER_PORT).toString();
    
    qDebug() << "配置的服务器IP:" << ip;
    qDebug() << "配置的服务器端口:" << port;
    
    if (ip.isEmpty() || port.isEmpty()) {
        qDebug() << "错误：服务器配置为空";
        QMessageBox::warning(this,"提示","请先配置服务器信息");
        ui->loginpushButton->setEnabled(true);
        ui->loginpushButton->setText("登录");
        return;
    }

    // 构建登录请求的URL - 修正为正确的API路径
    QUrl url(QString("http://%1:%2/api/login").arg(ip).arg(port));
    QNetworkRequest request(url);
    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构建JSON请求Body - 匹配服务器端期望的字段名
    QJsonObject jsonObj;
    jsonObj["userphone"] = userPhone;  // 服务器端期望 userphone
    jsonObj["password"] = password;
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);

    qDebug() << "发送登录请求到:" << url.toString();
    qDebug() << "请求数据:" << data;

    // 发送请求
    m_pLoginManager->post(request,data);
}




// 处理登录请求
void LoginDlg::on_loginReplyFinished(QNetworkReply *reply)
{
    qDebug() << "收到登录响应";
    
    // 恢复登录状态按钮
    ui->loginpushButton->setEnabled(true);
    ui->loginpushButton->setText("登录");
    
    // 处理网络错误
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "网络错误:" << reply->errorString();
        // 对话框显示错误
        QMessageBox::critical(this, "登录失败",QString("网络错误:%1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }
    // 无错误，解析响应数据
    // 读取响应数据
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (jsonDoc.isNull()) {
        QMessageBox::critical(this, "登录失败","服务器响应数据格式错误");
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    // 响应码
    int code = jsonObj["code"].toInt();
    // 消息
    QString message = jsonObj["msg"].toString();

    if ( code == 200 ) {
        // 保存用户信息到全局变量
        // 注意json的键
        g_stUserInfo.strUserPhone = jsonObj["userphone"].toString();
        g_stUserInfo.strUserId = jsonObj["userid"].toString();
        // 保存当前时间
        g_stUserInfo.strLoginTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        // 根据复选框状态保存密码
        if (ui->passwordcheckBox->isChecked()) {
            saveUserInfo(ui->phonelineEdit->text(), ui->passwordlineEdit->text());
        } else {
            saveUserInfo(ui->phonelineEdit->text(),"");
        }

        QMessageBox::information(this, "登录成功", message);
        accept(); // 关闭对话框

    } else {
        QMessageBox::critical(this, "登录失败", message);
    }
    reply->deleteLater();
}

// 点击注册按钮事件，调用注册对话框
void LoginDlg::on_registrypushButton_clicked()
{
    // 检查按钮是否存在
    if (!ui->registrypushButton) {
        qDebug() << "错误：registrypushButton 不存在！";
        return;
    }

    // 清空注册框并显示
    m_pRegisterDialog->show();
}



// 读取保存的用户信息
void LoginDlg::loadSavedUserInfo()
{
    // 配置信息加载
    QSettings settings;
    QString userPhone = settings.value(WEBSOCKET_USER_PHONE).toString();
    QString password = settings.value(WEBSOCKET_USER_PWD).toString();
    bool rememberPwd = settings.value(WEBSOCKET_REMBER_PWD).toBool();

    // 如果不为空，设置lineEdit内容
    if (!userPhone.isEmpty()) {
        ui->phonelineEdit->setText(userPhone);
    }
    if (rememberPwd && !password.isEmpty()) {
        ui->passwordlineEdit->setText(password);
        ui->passwordcheckBox->setChecked(true);
    }

}

// 保存用户信息
void LoginDlg::saveUserInfo(const QString &userPhone, const QString &password)
{
    QSettings settings;
    settings.setValue(WEBSOCKET_USER_PHONE, userPhone);
    settings.setValue(WEBSOCKET_USER_PWD, password);
    settings.setValue(WEBSOCKET_REMBER_PWD, ui->passwordcheckBox->isChecked());
}




void LoginDlg::on_serverpushButton_clicked()
{
    
    // 检查按钮是否存在
    if (!ui->serverpushButton) {
        qDebug() << "错误：serverpushButton 不存在！";
        return;
    }
    

    // 创建并显示服务器配置对话框
    SettingDlg settingDlg(this);
    
    if (settingDlg.exec() == QDialog::Accepted) {
        qDebug() << "用户确认了服务器配置";
        loadSavedUserInfo();
    } else {
        qDebug() << "用户取消了服务器配置";
    }
}

// 测试网络连接
//void LoginDlg::testNetworkConnection()
//{
//    QSettings settings;
//    QString ip = settings.value(CURRENT_SERVER_HOST).toString();
//    QString port = settings.value(WEBSOCKET_SERVER_PORT).toString();
    
//    if (!ip.isEmpty() && !port.isEmpty()) {
//        qDebug() << "测试网络连接到:" << ip << ":" << port;
        
//        // 创建一个简单的GET请求测试连接
//        QNetworkRequest request(QUrl(QString("http://%1:%2/").arg(ip).arg(port)));
//        QNetworkReply *reply = m_pLoginManager->get(request);
        
//        connect(reply, &QNetworkReply::finished, [this, reply]() {
//            if (reply->error() == QNetworkReply::NoError) {
//                qDebug() << "网络连接测试成功！";
//            } else {
//                qDebug() << "网络连接测试失败:" << reply->errorString();
//            }
//            reply->deleteLater();
//        });
//    }
//}


