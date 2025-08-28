#include "registrydlg.h"
#include "ui_registrydlg.h"



RegistryDlg::RegistryDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegistryDlg),
    m_pRegisterManager(nullptr)
{
    ui->setupUi(this);

    // 移除对话框右上角的问号按钮
    Qt::WindowFlags flags = this->windowFlags();
    setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle("用户注册");


    // 初始化网络注册管理器
    m_pRegisterManager = new QNetworkAccessManager(this);
    // 当完成注册请求之后，调用函数，接收响应
    connect(m_pRegisterManager, &QNetworkAccessManager::finished,
            this,&RegistryDlg::on_registerReplyFinished);


//    // 手动连接注册按钮的信号槽
//    connect(ui->registrypushButton, &QPushButton::clicked,
//            this, &RegistryDlg::on_registrypushButton_clicked);

//    // 手动连接关闭注册对话框的信号槽
//    connect(ui->cancelpushButton, &QPushButton::clicked,
//            this, &RegistryDlg::on_cancelpushButton_clicked);
}

RegistryDlg::~RegistryDlg()
{
    delete ui;
    if (m_pRegisterManager) delete m_pRegisterManager;
}



void RegistryDlg::on_cancelpushButton_clicked()
{
    // 立即恢复按钮状态，避免误锁定
    ui->registrypushButton->setEnabled(true);
    ui->registrypushButton->setText("注册");
    // 关闭对话框
    reject();
}

// 注册响应完成调用的函数
void RegistryDlg::on_registerReplyFinished(QNetworkReply *reply)
{
    // 恢复注册按钮状态
    ui->registrypushButton->setEnabled(true);
    ui->registrypushButton->setText("注册");

    // 处理网络响应返回错误
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(this,"注册失败",QString("网络错误:%1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    // 解析响应数据
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (jsonDoc.isNull()) {
        QMessageBox::critical(this, "注册失败","服务器响应格式错误");
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    int code = jsonObj["code"].toInt();
    QString message = jsonObj["message"].toString();
    if (code == 200) {
           QMessageBox::information(this, "注册成功",message);
           // 发送注册成功信号，将注册的手机号，通过信号携带出来，登录对话框直接读取
           emit registerSuccess(ui->phonelineEdit->text().trimmed());
           accept(); // 关闭注册对话框
    } else {
        QMessageBox::critical(this,"注册失败",message);
    }
    reply->deleteLater();
}

// 注册点击事件，并发送注册请求
void RegistryDlg::on_registrypushButton_clicked()
{
    QString errorMsg;
    if (!validateInput(errorMsg)){
           QMessageBox::warning(this, "输入错误",errorMsg);
                   return;
    }

    // 禁用按钮防止重复提交
    ui->registrypushButton->setEnabled(false);
    ui->registrypushButton->setText("注册中...");

    // 获取服务器配置
    QString ip, port;
    if (!getServerConfig(ip, port)) {
        ui->registrypushButton->setEnabled(true);
        ui->registrypushButton->setText("注册");
        return;
    }

    // 构建URL
    QUrl url(QString("http://%1:%2/api/register").arg(ip).arg(port));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // JSON请求body
    QJsonObject jsonObj;
    jsonObj["userphone"] = ui->phonelineEdit->text().trimmed();
    jsonObj["password"] = ui->passwordlineEdit->text().trimmed();
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);

    // 发送post请求
    m_pRegisterManager->post(request, data);
}

// 验证输入合法性
bool RegistryDlg::validateInput(QString &errorMsg)
{
    QString userPhone = ui->phonelineEdit->text().trimmed();
    QString password = ui->passwordlineEdit->text().trimmed();
    QString confirmPassword = ui->cfmpasswordlineEdit->text().trimmed();

    // 手机号验证
    if (userPhone.isEmpty() ) {
           errorMsg = "手机号不能为空";
           return false;
    }
    if (!IsValidPhoneNumber(userPhone)) {
            errorMsg = "手机号格式错误";
            return  false;
    }
    // 密码验证
    if (password.isEmpty()) {
        errorMsg = "密码不能为空";
        return  false;
    }
    if (password != confirmPassword) {
        errorMsg = "两次输入的密码不一致";
        return false;
    }
    return true;
}


// 验证手机号码
bool RegistryDlg::IsValidPhoneNumber(const QString & phoneNum)
{
    QRegExp rx("\\d+");
        QRegExp regx("^1([358][0-9]|4[579]|6[2567]|7[0135678]|9[0123589])[0-9]{8}$");
        QRegExpValidator regs(regx,0);
        QString d;
        int pos = 0;
        int nPos = 0;

        while ((pos = rx.indexIn(phoneNum, pos)) != -1) {
            QString strPhone = rx.cap(0);
            QValidator::State res = regs.validate(strPhone, nPos);
            if (QValidator::Acceptable == res) {
                return true;
            }
            pos += rx.matchedLength();
        }

}

// 获取服务器配置
bool RegistryDlg::getServerConfig(QString &ip, QString &port)
{
    QSettings settings;
    ip = settings.value(CURRENT_SERVER_HOST).toString();
    port = settings.value(WEBSOCKET_SERVER_PORT).toString();

    if(ip.isEmpty() || port.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先配置服务器信息");
        return false;
    }
    
    qDebug() << "获取服务器配置 - IP:" << ip << "Port:" << port;
    return true;
}



