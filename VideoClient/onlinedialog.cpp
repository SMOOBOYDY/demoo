#include "onlinedialog.h"
#include "ui_onlinedialog.h"
#include<QCryptographicHash>
#include<QMessageBox>
#include<QMovie>
#include<QFileInfo>
#define MD5_KEY  12345

static  QByteArray  GetMD5(QString val)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QString tmp = QString("%1_%2").arg(val).arg(MD5_KEY);
    hash.addData( tmp.toStdString().c_str() );
    QByteArray bt = hash.result();
    return bt.toHex();
}


OnlineDialog* OnlineDialog::m_online = NULL;
QString OnlineDialog::m_ServerIP = DEF_SERVER_IP;
OnlineDialog::OnlineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnlineDialog),m_id(0)
{
    ui->setupUi(this);

    this->setWindowTitle( "在线资源" );
    m_online = this;

    qsrand( QTime(0,0,0).msecsTo( QTime::currentTime() ) );
    qRegisterMetaType<Hobby>("Hobby");

    m_login = new LoginDialog();
    m_login->hide();
    connect( ui->pb_tv1 , SIGNAL(clicked()) , this , SLOT(slot_btnClicked()) );
    connect( ui->pb_tv2 , SIGNAL(clicked()) , this , SLOT(slot_btnClicked()) );
//    connect( ui->pb_play1 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play2 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play3 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play4 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play5 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play6 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play7 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play8 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play9 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );
//    connect( ui->pb_play10 , SIGNAL(SIG_labelClicked()) , this , SLOT(slot_PlayClicked()) );

    ui->pb_play1 ->installEventFilter(this);
    ui->pb_play2 ->installEventFilter(this);
    ui->pb_play3 ->installEventFilter(this);
    ui->pb_play4 ->installEventFilter(this);
    ui->pb_play5 ->installEventFilter(this);
    ui->pb_play6 ->installEventFilter(this);
    ui->pb_play7 ->installEventFilter(this);
    ui->pb_play8 ->installEventFilter(this);
    ui->pb_play9 ->installEventFilter(this);
    ui->pb_play10->installEventFilter(this);

    connect( m_login , SIGNAL(SIG_loginCommit(QString,QString)) ,
              this , SLOT(slot_loginCommit(QString,QString)) );
    connect( m_login , SIGNAL(SIG_registerCommit(QString,QString,Hobby)) ,
              this , SLOT(slot_registerCommit(QString,QString,Hobby)) );

    connect(this , SIGNAL(SIG_setMovie(int,QString,QString)) ,
            this , SLOT(slot_setMovie(int,QString,QString)) );

    InitServerIPFromIni();

    m_tcp = new QMyTcpClient();

    m_tcp->setIpAndPort( (char* )m_ServerIP.toStdString().c_str() );
    connect( m_tcp , SIGNAL(SIG_ReadyData(char*,int)) ,
             this , SLOT( slot_ReadyData(char*,int)) );

    m_uploadDlg = new UploadDialog;
//    connect( m_uploadDlg , SIGNAL( SIG_UploadFile(QString,QString,Hobby)) ,
//             this , SLOT( slot_UploadFile(QString,QString,Hobby)) );
    connect( this , SIGNAL( SIG_updateProcess(qint64,qint64) ) ,
             m_uploadDlg , SLOT( slot_updateProcess(qint64,qint64)) );
    connect( m_uploadDlg , SIGNAL( SIG_PlayRecord(QString)) ,
             this , SIGNAL(  SIG_PlayRecord(QString)) );
    m_uploadDlg->hide();

    m_uploadWorker = new UploadWorker;
    m_uploadThread = new QThread;
    connect( m_uploadDlg , SIGNAL( SIG_UploadFile(QString,QString,Hobby)) ,
             m_uploadWorker , SLOT( slot_UploadFile(QString,QString,Hobby)) );
    m_uploadWorker ->moveToThread( m_uploadThread );
    m_uploadThread ->start();

    m_downloadThread = new QThread;
    m_downloadWorker = new DownloadWorker;
    connect(this , SIGNAL(SIG_DownloadBlock(char*,int)),
            m_downloadWorker , SLOT(slot_downloadWork(char*,int)) );

    m_downloadWorker->moveToThread( m_downloadThread);
    m_downloadThread->start();

    ui->sw_page->setCurrentIndex( 0 );
}

OnlineDialog::~OnlineDialog()
{

    if( m_login )
    {
        m_login->close();
        delete m_login;
        m_login = NULL;
    }
    if( m_uploadDlg )
    {
        m_uploadDlg->close();
        delete m_uploadDlg;
        m_uploadDlg = NULL;
    }
    if( m_tcp )
    {
        delete m_tcp;
        m_tcp = NULL;
    }
    if( m_uploadWorker )
    {
        delete m_uploadWorker ; m_uploadWorker = NULL;
    }
    if( m_uploadThread )
    {
        m_uploadThread->quit();
        m_uploadThread->wait(100);
        if( m_uploadThread->isRunning() )
        {
            m_uploadThread->terminate();
            m_uploadThread->wait(100);
        }
        delete m_uploadThread ; m_uploadThread = NULL;
    }
    if( m_downloadWorker ){
        delete m_downloadWorker ;
        m_downloadWorker = NULL;
    }
    if( m_downloadThread ){ m_downloadThread->quit();
        m_downloadThread->wait(100);
        if( m_downloadThread ->isRunning() )
        {
            m_downloadThread->terminate();
        }
        delete m_downloadThread;
        m_downloadThread = NULL;
    }
    delete ui;
}
#include<QSettings>
//通过配置文件 初始化服务器ip地址 -- 路径与exe 同级路径下
void OnlineDialog::InitServerIPFromIni()
{
    qDebug()<< __func__;
    QFileInfo info( QCoreApplication::applicationDirPath()+"/serverip.ini" );

    if( !info.exists() )
    {
        QString arg1 = QCoreApplication::applicationDirPath()+"/serverip.ini"; //这个是你ini文件的路径
        QSettings setting(arg1,QSettings::IniFormat,NULL); // 打开ini文件
        setting.beginGroup("net");
        setting.setValue("ip" , DEF_SERVER_IP);
        qDebug()<< "SERVER_IP: "<< DEF_SERVER_IP;
        setting.endGroup();
    }else
    {
        //   1、创建对象
       QString arg1 = QCoreApplication::applicationDirPath()+"/serverip.ini"; //这个是你ini文件的路径
       QSettings setting(arg1,QSettings::IniFormat,NULL); // 打开ini文件

        //   2、读取内容
       setting.beginGroup("net");
       QVariant res = setting.value("ip");
       setting.endGroup();
       //    这样就会读取到key对应的value值。
       QString ip = res.toString();
       qDebug()<< ip;
       if( !ip.isEmpty() ) m_ServerIP = ip;
    }
}

// 点击 登录/注册
void OnlineDialog::on_pb_login_clicked()
{
    m_login->show();
}

//电视节目点击响应
void OnlineDialog::slot_btnClicked()
{
    if(QObject::sender() == ui->pb_tv1)
    {
        emit SIG_PlayTV( OnlineDialog::HuNanTv);
    }else if(QObject::sender() == ui->pb_tv2)
    {
        emit SIG_PlayTV( OnlineDialog::GuangXiTv);
    }

}

//登录提交  MD5
void OnlineDialog::slot_loginCommit(QString name, QString password)
{
    m_userName = name;
    std::string strName = name.toStdString();
    char* bufName = (char*)strName.c_str();

    QByteArray bt = GetMD5( password );

    STRU_LOGIN_RQ rq;
    strcpy_s(  rq.m_szUser ,    MAX_SIZE , bufName);
    memcpy( rq.m_szPassword , bt.data() , bt.length()  );
    int ping=m_tcp->SendData( (char*)&rq , sizeof(rq) );
    printf("%d\n",ping);
    if(   ping< 0  )
    {
        QMessageBox::about( this , "提示","网络故障");
    }
}
//注册提交
void OnlineDialog::slot_registerCommit(QString name, QString password, Hobby hy)
{
    std::string strName = name.toStdString();
    char* bufName = (char*)strName.c_str();

    QByteArray bt = GetMD5( password );

    STRU_REGISTER_RQ rq;
    strcpy_s(  rq.m_szUser ,    MAX_SIZE , bufName);
    memcpy( rq.m_szPassword , bt.data() , bt.length()  );

    rq.dance   = hy.dance   ;
    rq.edu     = hy.edu     ;
    rq.ennegy  = hy.ennegy  ;
    rq.food    = hy.food    ;
    rq.funny   = hy.funny   ;
    rq.music   = hy.music   ;
    rq.outside = hy.outside ;
    rq.video   = hy.video   ;

    if(  m_tcp->SendData( (char*)&rq , sizeof(rq) ) < 0  )
    {
        QMessageBox::about( this , "提示","网络故障");
    }
}
//tcp网络接收
void OnlineDialog::slot_ReadyData(char *buf, int nlen)
{
    int nType =  *(int*) buf;
    switch( nType )
    {
        case DEF_PACK_LOGIN_RS:
            slot_loginRs( buf,   nlen);
            delete [] buf;
        break;
        case DEF_PACK_REGISTER_RS:
            slot_registerRs( buf,   nlen);
            delete [] buf;
        break;
        case DEF_PACK_UPLOAD_RS:
            slot_uploadRs( buf,   nlen);
            delete [] buf;
        break;
        case DEF_PACK_DOWNLOAD_RS:
            slot_downloadRs( buf,   nlen);
            delete [] buf;
        break;
        case DEF_PACK_FILEBLOCK_RQ:
            //slot_fileBlockRq( buf,   nlen);
            Q_EMIT SIG_DownloadBlock( buf, nlen);
        break;
    }
    //delete [] buf;
}
//用户登录回复
void OnlineDialog::slot_loginRs(char *buf, int nlen)
{

    STRU_LOGIN_RS *rs = (STRU_LOGIN_RS *) buf;
    switch( rs ->m_lResult )
    {
        case userid_no_exist :
            QMessageBox::about( m_login , "提示" , "用户不存在, 登录失败");
        break;
        case password_error :
            QMessageBox::about( m_login , "提示" , "用户密码错误, 登录失败");
        break;
        case login_sucess :
            QMessageBox::about( m_login , "提示" , "登录成功");
            //界面
            ui->lb_name->setText( QString("您好, %1") .arg(m_userName));
            m_login->hide();
            m_id = rs->m_UserID ;

            //下载列表文件
            STRU_DOWNLOAD_RQ rq;
            rq.m_nUserId = m_id;
            m_tcp->SendData( (char*)&rq , sizeof(rq));

        break;
    }
}
//用户注册回复
void OnlineDialog::slot_registerRs(char *buf, int nlen)
{
    STRU_REGISTER_RS * rs = (STRU_REGISTER_RS * ) buf;
    switch( rs->m_lResult )
    {
        case userid_is_exist:
            QMessageBox::about( m_login , "提示" , "用户已存在, 注册失败");
        break;
        case register_sucess:
            QMessageBox::about( m_login , "提示" , "注册成功");
        break;
    }
}


//上传回复
void OnlineDialog::slot_uploadRs(char *buf, int nlen)
{
    STRU_UPLOAD_RS * rs = (STRU_UPLOAD_RS * ) buf;
    switch( rs->m_nResult )
    {
        case 1:
            QMessageBox::about( m_login , "提示" , "上传成功");
        break;

    }
}
#include<QDir>
//处理下载回复
void OnlineDialog::slot_downloadRs(char *buf, int nlen)
{
    STRU_DOWNLOAD_RS * rs = (STRU_DOWNLOAD_RS *)buf;

    //文件头

    //给FileInfo 赋值

    FileInfo * info = new FileInfo;
    // videoid 作为文件的标示 , fileid 用来区分不同控件
    info->videoId = rs->m_nVideoId;
    info->fileId = rs->m_nFileId;
    info->fileName = rs->m_szFileName;

//    (./temp/);
    QDir dir;
    if( !dir.exists( QDir::currentPath() +"/temp/"))
    {
        dir.mkpath(QDir::currentPath() +"/temp/");
    }
    info->filePath = QString("./temp/%1").arg(rs->m_szFileName);

    info->filePos = 0;
    info->fileSize = rs->m_nFileSize;

    info->rtmpUrl = QString( "rtmp://%1/vod%2").arg(OnlineDialog::m_ServerIP).arg( rs->m_rtmp ); //  //1/104.mp4

    qDebug()<< "rtmp -- "<< info->rtmpUrl;
    // rtmp://192.168.0.10/vod//1/104.mp4
    info->pFile = new QFile(info->filePath);

    if( info->pFile->open(QIODevice::WriteOnly))
    {
        m_mapVideoIDTOFileInfo[ info->videoId ] = info;
    }else
    {
        delete info;
    }
}

//下载文件块
void OnlineDialog::slot_fileBlockRq(char *buf, int nlen)
{
    STRU_FILEBLOCK_RQ * rq = (STRU_FILEBLOCK_RQ *)buf;

    auto ite = m_mapVideoIDTOFileInfo.find( rq->m_nFileId );
    if(  ite ==  m_mapVideoIDTOFileInfo.end())  return;

    FileInfo* info =    m_mapVideoIDTOFileInfo[ rq->m_nFileId ];

    int64_t res =  info->pFile->write( rq->m_szFileContent , rq->m_nBlockLen );
    info->filePos += res;

    if( info->filePos >= info->fileSize )
    {
        //关闭文件
        info->pFile->close();
        //删除该节点
        m_mapVideoIDTOFileInfo.erase( ite );
        //设置到控件
        //info->fileId --> 控件号码

        Q_EMIT SIG_setMovie( info->fileId , info->filePath , info->rtmpUrl);
////////////////////////////
//        QString pbNum = QString("pb_play%1").arg(info->fileId);
//        QLabel * pb_play = ui->sw_page->findChild<QLabel *>(pbNum);

//        QMovie *LastMovie = pb_play->movie();
//        if( LastMovie && LastMovie->isValid() )
//        {
//            delete LastMovie;
//        }
//        QMovie *movie = new QMovie(info->filePath);
//        movie->setScaledSize( QSize(150,120 ) );

//        pb_play->setMovie(movie);
//        pb_play->start();
//        pb_play->stop();
//        pb_play->setRtmpUrl( info->rtmpUrl);
////////////////////////////////////////////

        //回收info
        delete info;
        info = NULL;
    }
}

void OnlineDialog::slot_setMovie(int id , QString path , QString rtmp)
{
    //load to widget

//  qDebug()<< path;
    QMovie * movie = new QMovie(path);
    movie->setScaledSize( QSize(150,120 ) );
    QString pushbuttonName = QString("pb_play%1").arg(id  );

//    QMyMovieLabel* pb_play =  ui->page_2->findChild<QMyMovieLabel*>( pushbuttonName);
    QLabel* pb_play =  ui->sw_page->findChild<QLabel*>( pushbuttonName);
    //先清理再加载
    QMovie *lastMovie = pb_play->movie();

    pb_play->setMovie(movie);
    movie->start();
    movie->stop();
//    qDebug()<< rtmp;
//    pb_play->setRtmpUrl( rtmp );

    m_mapButtonNameToRtmpUrl[pushbuttonName] =  rtmp;
    if( lastMovie&&lastMovie->isValid() ){
        delete lastMovie;
    }
}

//点播项被点击
void OnlineDialog::slot_PlayClicked()
{
//    QLabel * pb_play = (QLabel *)QObject::sender();

//    pb_play->objectName();
//    Q_EMIT SIG_PlayUrl(  pb_play->rtmpUrl() );
}

//上传文件响应
void OnlineDialog::slot_UploadFile(QString filePath, QString imgPath, Hobby hy)
{
    //上传
    qDebug()<< "上传开始";
    UploadFile( imgPath , hy );
    UploadFile( filePath , hy , imgPath );

}


//上传文件
void OnlineDialog::UploadFile(QString filePath, Hobby hy , QString gifName  )
{
    //兼容中文
    QFileInfo info(filePath);
    QString FileName = info.fileName();
    std::string strName = FileName.toStdString();
    const char* file_name = strName.c_str();

    
    STRU_UPLOAD_RQ rq;
    rq.m_nFileId = qrand()%10000;
    rq.m_nFileSize = info.size();
    strcpy_s( rq.m_szFileName , MAX_PATH , file_name );
    
    QByteArray bt =  filePath.right( filePath.length() - filePath.lastIndexOf('.') -1 ).toLatin1();
    
    memcpy(rq.m_szFileType  ,bt.data() , bt.length() );
    
    if( !gifName.isEmpty() )
    {
        QFileInfo info(gifName);
        strcpy_s( rq.m_szGifName , MAX_PATH  , info.fileName().toLocal8Bit().data() );
    }
    memcpy(  rq.m_szHobby , &hy ,sizeof(hy) );
    rq.m_UserId = m_id;
    
    m_tcp->SendData( (char*)&rq , sizeof( rq ));
    
    FileInfo  fi;
    fi.fileId = rq.m_nFileId;
    fi.fileName = FileName;
    fi.filePath = filePath;
    fi.filePos = 0;
    fi.fileSize = rq.m_nFileSize;
    fi.pFile = new QFile(filePath);  
    
    if( fi.pFile->open(QIODevice::ReadOnly))
    {
        while(1)
        {
            STRU_FILEBLOCK_RQ blockrq;
            
             int64_t res = fi.pFile->read( blockrq.m_szFileContent , MAX_CONTENT_LEN);
             fi.filePos += res;
             blockrq.m_nBlockLen = res ;
             blockrq.m_nFileId = rq.m_nFileId;
             blockrq.m_nUserId = m_id;
             
             m_tcp->SendData( (char* ) &blockrq , sizeof( blockrq));
             emit SIG_updateProcess( fi.filePos , fi.fileSize);
             
             if( fi.filePos >= fi.fileSize )
             {
                 fi.pFile->close();
                 delete fi.pFile;
                 fi.pFile = NULL;
                 break;
             }
        }
    }
}


//点击上传视频
void OnlineDialog::on_pb_upload_clicked()
{
    if( m_id == 0 )
    {
        QMessageBox::about( this , "提示" , "先登录");
        return;
    }
    m_uploadDlg->clear();
    m_uploadDlg->show();
}

//事件过滤器
bool OnlineDialog::eventFilter(QObject *watch, QEvent *event)
{
    if( watch == ui->pb_play1 || watch == ui->pb_play2 ||
        watch == ui->pb_play3 || watch == ui->pb_play4 ||
        watch == ui->pb_play5 || watch == ui->pb_play6 ||
        watch == ui->pb_play7 || watch == ui->pb_play8 ||
        watch == ui->pb_play9 || watch == ui->pb_play10  )
    {
        QLabel * label = (QLabel*) watch;
        if( event->type() == QEvent::Enter )
        {

            if( label->movie() )
                label->movie()->start();
        }else if( event->type() == QEvent::Leave)
        {

            if( label->movie() )
                label->movie()->stop();
        }else if( event->type() == QEvent::MouseButtonPress )
        {
            QString name = label->objectName();
            Q_EMIT SIG_PlayUrl( m_mapButtonNameToRtmpUrl[name] );
            //Q_EMIT SIG_PlayRecord( );
        }
    }
    return QObject::eventFilter(watch, event);
}

//工作者 上传流程
void UploadWorker::slot_UploadFile(QString filePath, QString imgPath, Hobby hy)
{
    OnlineDialog::m_online->slot_UploadFile( filePath , imgPath , hy );
}

//地方台分页
void OnlineDialog::on_pb_tvShow_clicked()
{
    ui->sw_page->setCurrentIndex( 0 );
}

//点播分页
void OnlineDialog::on_pb_video_clicked()
{
    if( !m_id )
    {
        QMessageBox::about(this, "提示" , "先登录")   ;
        return;
    }
    ui->sw_page->setCurrentIndex( 1 );
}


//刷新
void OnlineDialog::on_pb_fresh_clicked()
{
    if( !m_id )
    {
        QMessageBox::about(this, "提示" , "先登录")   ;
        return;
    }

    STRU_DOWNLOAD_RQ rq;
    rq.m_nUserId = m_id ;
    m_tcp ->SendData( (char*)&rq , sizeof(rq));
}

//播放自定义url
void OnlineDialog::on_pb_rtmpPlay_clicked()
{
    if( ui->le_rtmpUrl->text()  == "输入rtmp地址" )
    {
        QMessageBox::about(this , "提示" , "输入rtmp地址");
        return;
    }
    Q_EMIT SIG_PlayUrl( ui->le_rtmpUrl->text() );
}

//下载文件工作者
void DownloadWorker::slot_downloadWork(char *buf, int nlen)
{
    OnlineDialog::m_online->slot_fileBlockRq(buf , nlen);
    delete [] buf;
}
