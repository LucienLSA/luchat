#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>

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

private:
    Ui::ChatWidget *ui;

    QTextEdit *m_pTextEdit; // 消息显示区域

    bool m_bIsMainWindow;  // 标记是否为主窗口（群聊窗口）

    // 消息模板（HTML格式，方便格式化显示）
    QString m_strContentTemplateWithLink;  // 带文件链接的消息模板
    QString m_strContentTemplateWithoutLink;  // 无链接的消息模板
};

#endif // CHATWIDGET_H
