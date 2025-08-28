#include "passwordedit.h"

PasswordEdit::PasswordEdit(QWidget *parent):QLineEdit(parent)
{
    init();
}

// 初始化
void PasswordEdit::init()
{
    // 初始不可见
    m_bVisible = false;
    setEchoMode(QLineEdit::Password);

    // 切换按钮
    m_pAction = new QAction(this);
    m_pAction->setIcon(QIcon(":/tools/images/invisible.svg"));
    addAction(m_pAction,QLineEdit::TrailingPosition);//右侧显示
    // 当点击切换按钮的图片时，调用函数
    connect(m_pAction,&QAction::triggered,this,&PasswordEdit::onInputVisible);
}

// 设置密码可见
void PasswordEdit::onInputVisible()
{
    // 取反
    m_bVisible = !m_bVisible;
    // 如果可见，设置图片和LineEdit
    if(m_bVisible)
    {
        m_pAction->setIcon(QIcon(":/tools/images/visible.svg"));
        setEchoMode(QLineEdit::Normal);
    }
    else
    {
        m_pAction->setIcon(QIcon(":/tools/images/invisible.svg"));
        setEchoMode(QLineEdit::Password);
    }
}
