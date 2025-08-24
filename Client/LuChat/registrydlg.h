#ifndef REGISTRYDLG_H
#define REGISTRYDLG_H

#include <QDialog>

namespace Ui {
class RegistryDlg;
}

class RegistryDlg : public QDialog
{
    Q_OBJECT

public:
    explicit RegistryDlg(QWidget *parent = nullptr);
    ~RegistryDlg();

private:
    Ui::RegistryDlg *ui;
};

#endif // REGISTRYDLG_H
