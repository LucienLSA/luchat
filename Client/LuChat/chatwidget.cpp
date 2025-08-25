#include "chatwidget.h"
#include "ui_chatwidget.h"

ChatWidget::ChatWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWidget),
    m_pTextEdit(nullptr)
{
     m_bIsMainWindow = true;
    ui->setupUi(this);
    // 1. 初始化消息输入框
    ui->inputTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);  // 按窗口宽度自动换行
    // 2. 初始化群聊消息显示区域（默认标签页）
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setReadOnly(true);  // 消息区域只读
    m_pTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);

    // 设置大小策略（拉伸比例）
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHorizontalStretch(4);
    policy.setVerticalStretch(3);
    m_pTextEdit->setSizePolicy(policy);

    // 3. 初始化标签页（QTabWidget）
    // 清除默认标签页
    int nCount = ui->showMsgTabWidget->count();
    for (int i = 0; i < nCount; ++i) {
        ui->showMsgTabWidget->removeTab(0);
    }
    // 添加群聊标签页
    ui->showMsgTabWidget->addTab(m_pTextEdit, "聊天窗口");
    ui->showMsgTabWidget->setTabsClosable(true);  // 标签页可关闭（除了群聊）
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

    // 6. 禁用上传按钮（默认未选择文件时不可用）
//    ui->uploadFilePushButton->setDisabled(true);

    // 7. 初始化消息模板（HTML格式）
    m_strContentTemplateWithoutLink = "<p><a>%1:</a><br />&nbsp;&nbsp;&nbsp;&nbsp;<a>%2</a>&nbsp;&nbsp;&nbsp;&nbsp;<a>(%3)</a></p>";
    m_strContentTemplateWithLink = "<p><a>%1:</a><br />&nbsp;&nbsp;&nbsp;&nbsp;<a>%2</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"%3\">%4</a>&nbsp;&nbsp;&nbsp;&nbsp;<a>(%5)</a></p>";
}

ChatWidget::~ChatWidget()
{
    delete ui;
}
