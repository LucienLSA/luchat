#include "settingdlg.h"
#include "ui_settingdlg.h"

SettingDlg::SettingDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDlg)
{
    ui->setupUi(this);
}

SettingDlg::~SettingDlg()
{
    delete ui;
}
