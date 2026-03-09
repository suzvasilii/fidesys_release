#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(862, 697);
    w.setWindowTitle("Fidesys processing");
    w.show();
    return a.exec();
}
