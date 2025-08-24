#include "registrydlg.h"
#include "ui_registrydlg.h"

RegistryDlg::RegistryDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegistryDlg)
{
    ui->setupUi(this);
}

RegistryDlg::~RegistryDlg()
{
    delete ui;
}
