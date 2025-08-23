#include "mainwindow.h"

#include <QApplication>
#include "common.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化应用程序目录
    APPLICATION_DIR = QApplication::applicationDirPath();
    qDebug() << "应用目录" << APPLICATION_DIR;

    // 时间格式化函数
    qDebug() << "当前时间" << FormatTime();

    MainWindow w;
    w.show();
    return a.exec();
}
