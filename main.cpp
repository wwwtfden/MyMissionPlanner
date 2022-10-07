#include "mainwindow.h"

#include "cachepainter.h"
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QPainter>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
