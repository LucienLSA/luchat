#ifndef SETTINGDLG_H
#define SETTINGDLG_H

#include <QDialog>

namespace Ui {
class SettingDlg;
}

class SettingDlg : public QDialog
{
    Q_OBJECT

public:
    static SettingDlg *GetInstance() {
        if (m_pInstance == nullptr) {
            m_pInstance = new SettingDlg();
        }

        return m_pInstance;
    }
    ~SettingDlg();
private:
    explicit SettingDlg(QWidget *parent = nullptr);

private slots:
    void on_okpushButton_clicked();
    void on_cancelpushButton_clicked();

private:
    Ui::SettingDlg *ui;
    static SettingDlg *m_pInstance;
};

#endif // SETTINGDLG_H
