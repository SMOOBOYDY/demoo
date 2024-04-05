#include "videoshow.h"
#include "customtittledialog.h"
#include <QApplication>

#include<QFile>

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    VideoShow w;
//    w.show();

    QFile file(QString(":/qss/gray.css"));//灰色风格
    file.open(QFile::ReadOnly);
    QString qss = QLatin1String(file.readAll());
    qApp->setStyleSheet(qss);


    CustomTittleDialog window;
    window.show();

    return a.exec();
}
