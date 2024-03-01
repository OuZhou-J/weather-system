#include "weatherwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    weatherwindow w;
    w.show();
    return a.exec();
}
