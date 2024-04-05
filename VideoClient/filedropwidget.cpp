#include "filedropwidget.h"

#include <QDir>
#include <QFile>
#include <QDropEvent>
#include <QMimeData>
QFileDropWidget::QFileDropWidget(QWidget *parent) : QWidget(parent)
{
    setAcceptDrops(true);
}

bool QFileDropWidget::IsValidDragFile(QDropEvent *e)
{
    QString text = e->mimeData()->text();
    bool bre = false;
    do
    {
        // 存在多个文件时，只判断第一个
        QStringList files = text.split("\n");
        if(files.size() < 1)
        {
            break;
        }

        int pos = files[0].indexOf("file:///");
        if(0 != pos)
        {
            break;
        }

        //去掉拖拽头标 file:///
        QString filename = files[0].mid(8);

        // 接受目录
        QDir dir(filename);
        if (dir.exists())
        {
            bre = true;
            break;
        }

        if(QFile::exists(filename))
        {
            bre = true;
            break;
        }

    }while(false);

    return bre;
}

//void QFileDropWidget::AcceptFolder(QString folder)
//{
//    QDir dir(folder);

//    QFileInfo info(folder);
//    QString Path = info.absoluteFilePath();
//    QString Name = Path.right( Path.length() - Path.lastIndexOf('/') -1 );

//    emit SIG_FileComing(Name, Path );
//}

void QFileDropWidget::AcceptFile(QString pathfile)
{

    QFileInfo info(pathfile);
    QString Path = info.absoluteFilePath();
    QString Name = Path.right( Path.length() - Path.lastIndexOf('/') -1 );

    emit SIG_FileComing(Name, Path );
}

void QFileDropWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(IsValidDragFile(event))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void QFileDropWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if(IsValidDragFile(event))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void QFileDropWidget::dropEvent(QDropEvent *event)
{
    QStringList files = event->mimeData()->text().split("\n");
    for(int i = 0; i < files.size(); i++)
    {
        QString fileName = files[i].mid(8); //  去掉头file:///

        // 接受目录
        QDir dir(fileName);
        if (dir.exists())
        {
            AcceptFile(fileName);
            continue;
        }

        if(QFile::exists(fileName))
        {
            AcceptFile(fileName);
            continue;
        }
    }
}
