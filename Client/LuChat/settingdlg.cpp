#include "settingdlg.h"
#include "ui_settingdlg.h"
#include <QSettings>
#include "common.h"
#include <QMessageBox>

// Define the static singleton instance pointer
SettingDlg *SettingDlg::m_pInstance = nullptr;

SettingDlg::SettingDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDlg)
{
    // 移除对话框右上角的问号按钮
    Qt::WindowFlags flags = this->windowFlags();
    setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    
    // 设置对话框标题
    setWindowTitle("设置");
    // setupUi 已自动调用 connectSlotsByName

    // 从配置中读取已保存的IP和端口并显示
    QSettings setting;
    QString host = setting.value(CURRENT_SERVER_HOST, "").toString();
    QString port = setting.value(WEBSOCKET_SERVER_PORT,"").toString();
    if (!host.isEmpty()) {
        ui->iplineEdit->setText(host);
    }
    if (!port.isEmpty()) {
        ui->portlineEdit->setText(port);
    }

//    connect(ui->okpushButton, &QPushButton::clicked, this,
//            &SettingDlg::on_okpushButton_clicked);

//    connect(ui->cancelpushButton, &QPushButton::clicked, this,
//            &SettingDlg::on_cancelpushButton_clicked);
}

SettingDlg::~SettingDlg()
{
    delete ui;
}

// 验证配置信息格式并保存
void SettingDlg::on_okpushButton_clicked()
{
    QString ip = ui->iplineEdit->text().trimmed();
    QString port = ui->portlineEdit->text().trimmed();

    // 判断空
    if (ip.isEmpty() || port.isEmpty()) {
        QMessageBox::warning(this, "提示","IP或者PORT不能为空");
        return;
    }
    // 验证IP地址格式
    QStringList ipParts = ip.split(".");
    if (ipParts.size() != 4) {
        QMessageBox::warning(this, "提示", "IP地址格式错误");
        return;
    }
    // 遍历IP地址判断
    for ( const QString &part:ipParts) {
        bool isNumber;
        // 将IP地址中字符串转化为Int
        int num = part.toInt(&isNumber);
        if(!isNumber || num < 0 || num > 255) {
            QMessageBox::warning(this, "提示","IP地址中包含无效数字");
            return;
        }
    }
    // 验证端口格式
    bool isNumber;
    int portnum = port.toInt(&isNumber);
    if (!isNumber || portnum < 0 || portnum > 65535)  {
        QMessageBox::warning(this, "提示", "PORT端口号包含无效数字");
        return ;
    }

    // 保存配置
    QSettings setting;
    QString oldIp = setting.value(CURRENT_SERVER_HOST).toString();
    QString oldPort = setting.value(WEBSOCKET_SERVER_PORT).toString();

    // 如果配置发生变化，提示需要重启程序
    bool needRestart = false;
    if ((!oldIp.isEmpty() || oldPort.isEmpty()) && (oldIp != ip || oldPort != port)) {
        QMessageBox::information(this,"提示","配置文件已经被修改，需要重启程序生效");
        needRestart = true;
    }

    // 保存新配置
    setting.setValue(CURRENT_SERVER_HOST, ip);
    setting.setValue(WEBSOCKET_SERVER_PORT, port);

    // 如果需要重启，使用工具函数
    if (needRestart == true ) {
        RestartApp();
        exit(-1);
    } 
    accept();
}

void SettingDlg::on_cancelpushButton_clicked()
{
    reject();
}
