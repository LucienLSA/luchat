#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include <QLineEdit>
#include <QAction>

class PasswordEdit:public QLineEdit
{
public:
    explicit PasswordEdit(QWidget *parent = nullptr);
private:
    void init();
    void onInputVisible();

private:
    QAction *m_pAction;
    bool m_bVisible;

};

#endif // PASSWORDEDIT_H
