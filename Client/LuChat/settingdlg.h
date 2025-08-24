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
    explicit SettingDlg(QWidget *parent = nullptr);
    ~SettingDlg();

private slots:
    void on_okpushButton_clicked();

    void on_cancelpushButton_clicked();

private:
    Ui::SettingDlg *ui;
};

#endif // SETTINGDLG_H
