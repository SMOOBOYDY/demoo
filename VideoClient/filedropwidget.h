#ifndef QFILEDROPWIDGET_H
#define QFILEDROPWIDGET_H

#include <QWidget>

class QFileDropWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QFileDropWidget(QWidget *parent = 0);


protected:
    ///判断是否为有效的文件
    virtual bool IsValidDragFile(QDropEvent *e);

    ///接受目录
    /// @note 遍例目录，调用AcceptFile
//    virtual void AcceptFolder(QString folder);

    ///接受文件
    virtual void AcceptFile(QString pathfile);

// QWidget interface
protected:
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dragMoveEvent(QDragMoveEvent *e) override;
    virtual void dropEvent(QDropEvent *e) override;

signals:
    void SIG_FileComing(QString FileName  , QString Path );
public slots:

};

#endif // QFILEDROPWIDGET_H
