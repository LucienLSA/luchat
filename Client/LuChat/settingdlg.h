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

private:
    Ui::SettingDlg *ui;
};

#endif // SETTINGDLG_H
