#ifndef CUSTOMTITTLEDIALOG_H
#define CUSTOMTITTLEDIALOG_H

#include <QDialog>
#include<QCloseEvent>
#include"customwidget.h"

namespace Ui {
class CustomTittleDialog;
}

class CustomTittleDialog : public CustomMoveDialog
{
    Q_OBJECT

public:
    explicit CustomTittleDialog(QWidget *parent = 0);
    ~CustomTittleDialog();

public slots:
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void on_pb_menu_clicked();

    void on_pb_min_clicked();

    void on_pb_max_clicked();

    void on_pb_close_clicked();

    void closeEvent(QCloseEvent * event);

private:
    Ui::CustomTittleDialog *ui;
};

#endif // CUSTOMTITTLEDIALOG_H
