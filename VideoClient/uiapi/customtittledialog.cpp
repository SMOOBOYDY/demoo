#include "customtittledialog.h"
#include "ui_customtittledialog.h"

CustomTittleDialog::CustomTittleDialog(QWidget *parent) :
    CustomMoveDialog(parent),
    ui(new Ui::CustomTittleDialog)
{
    ui->setupUi(this);

    setWindowTitle("我的播放器");

    //安装事件监听器,让标题栏识别鼠标双击
    ui->lb_tittle->installEventFilter(this);
}

CustomTittleDialog::~CustomTittleDialog()
{
    delete ui;
}

bool CustomTittleDialog::eventFilter(QObject *obj, QEvent *event)
{
    if ( obj == ui->lb_tittle && event->type() == QEvent::MouseButtonDblClick) {
             this->on_pb_max_clicked();
        return true;
    }
    return QObject::eventFilter( obj, event);
}

//菜单按钮
void CustomTittleDialog::on_pb_menu_clicked()
{

}

//最小化
void CustomTittleDialog::on_pb_min_clicked()
{
    this->slot_showMin();
}

//最大化
void CustomTittleDialog::on_pb_max_clicked()
{
    this->slot_showMax();
}

//关闭
void CustomTittleDialog::on_pb_close_clicked()
{
    this->slot_close();
}

//关闭事件
void CustomTittleDialog::closeEvent(QCloseEvent *event)
{

    //回收资源
    ui->wdg_videoShow->clear();

    event->accept();
}
