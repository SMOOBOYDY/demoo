#include<TCPKernel.h>
#include "packdef.h"
#include<stdio.h>
#include<sys/time.h>

using namespace std;


////注册
//#define  DEF_PACK_REGISTER_RQ    (DEF_PACK_BASE + 0)
//#define  DEF_PACK_REGISTER_RS    (DEF_PACK_BASE + 1)
////登录
//#define  DEF_PACK_LOGIN_RQ    (DEF_PACK_BASE + 2)
//#define  DEF_PACK_LOGIN_RS    (DEF_PACK_BASE + 3)

static const ProtocolMap m_ProtocolMapEntries[] =
{
    {DEF_PACK_REGISTER_RQ , &TcpKernel::RegisterRq},
    {DEF_PACK_LOGIN_RQ , &TcpKernel::LoginRq},
    {DEF_PACK_UPLOAD_RQ , &TcpKernel::UploadRq},
    {DEF_PACK_FILEBLOCK_RQ , &TcpKernel::UploadBlockRq},
    {DEF_PACK_DOWNLOAD_RQ , &TcpKernel::DownloadRq},
    {0,0}
};
#define RootPath   "/home/colin/video/"


int TcpKernel::Open()
{
    m_sql = new CMysql;
    m_tcp = new TcpNet(this);
    m_tcp->SetpThis(m_tcp);
    pthread_mutex_init(&m_tcp->alock,NULL);
    pthread_mutex_init(&m_tcp->rlock,NULL);
    if(  !m_sql->ConnectMysql("localhost","root","colin123","VideoServer")  )
    {
        printf("Conncet Mysql Failed...\n");
        return FALSE;
    }
    else
    {
        printf("MySql Connect Success...\n");
    }
    if( !m_tcp->InitNetWork()  )
    {
        printf("InitNetWork Failed...\n");
        return FALSE;
    }
    else
    {
        printf("Init Net Success...\n");
    }

    return TRUE;
}

void TcpKernel::Close()
{
    for( auto ite = m_mapFileIDToFileInfo.begin();ite!=m_mapFileIDToFileInfo.end();++ite)
    {
        delete ite ->second;
    }
    m_mapFileIDToFileInfo.clear();

    m_mapIDToUserFD.clear();
    m_sql->DisConnect();
    m_tcp->UnInitNetWork();
}


void TcpKernel::DealData(int clientfd,char *szbuf,int nlen)
{
    PackType *pType = (PackType*)szbuf;
    int i = 0;
    while(1)
    {
        if(*pType == m_ProtocolMapEntries[i].m_type)
        {
            auto fun= m_ProtocolMapEntries[i].m_pfun;
            (this->*fun)(clientfd,szbuf,nlen);
        }
        else if(m_ProtocolMapEntries[i].m_type == 0 &&
                m_ProtocolMapEntries[i].m_pfun == 0)
            return;
        ++i;
    }
    return;
}

//用户表  t_UserData
//表的字段有 id , name , password , food , funny ,ennegy ,dance , music,  video,  outside , edu

//注册
void TcpKernel::RegisterRq(int clientfd,char* szbuf,int nlen)
{
    printf("clientfd:%d RegisterRq\n", clientfd);

    STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ *)szbuf;
    STRU_REGISTER_RS rs;

    char sqlBuf[_DEF_SQLIEN] ="";
    sprintf( sqlBuf , "select name from t_UserData where name= '%s';", rq->m_szUser);
    list<string> resList;
    bool res = m_sql->SelectMysql( sqlBuf ,1 , resList);
    if(!res )
    {
        cout<< "SelectMysql error: "<<sqlBuf <<endl;
        return;
    }
////    //注册请求结果
////    #define userid_is_exist      0
////    #define register_sucess      1
    //查表 查name
    if( resList.size() > 0 )
    {
        //有  不成功
        rs.m_lResult = userid_is_exist;
    }else
    {//没有 写表 成功
        char sqlBuf[_DEF_SQLIEN] ="";
        sprintf( sqlBuf , "insert into t_UserData (name ,password ,food , funny ,ennegy ,dance , "
                          "music,  video,  outside , edu) values('%s','%s', %d, %d, %d, %d, %d, %d, %d, %d);",
                 rq->m_szUser,rq->m_szPassword ,
                 rq->food , rq->funny ,rq->ennegy ,rq->dance , rq->music,  rq->video,  rq->outside ,rq-> edu);
        m_sql->UpdataMysql(sqlBuf);

        sprintf( sqlBuf , "select id from t_UserData where name ='%s'",rq->m_szUser);
        list<string > resID;
        m_sql->SelectMysql(sqlBuf , 1,resID );
        int id = 0;
        if( resID.size() > 0)
        {
            id = atoi( resID.front().c_str() );
        }
       // rs.m_id = id;
        //新注册的用户 给创建一个路径
        char path[MAX_PATH] = "";
        sprintf(path , "%sflv/%s/" ,RootPath , rq->m_szUser );  // home/colin/video/flv/userName

        umask(0);
        mkdir(path , S_IRWXU|S_IRWXG|S_IRWXO );
        // S_IRWXU 00700权限，代表该文件所有者拥有读，写和执行操作的权限
        // S_IRWXG 00070权限，代表该文件用户组拥有读，写和执行操作的权限
        // S_IRWXO 00007权限，代表其他用户拥有读，写和执行操作的权限

        rs.m_lResult = register_sucess;
    }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}
//登录
void TcpKernel::LoginRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);

    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
    STRU_LOGIN_RS rs;
////    //登录请求结果
////    #define userid_no_exist      0
////    #define password_error       1
////    #define login_sucess         2
////    #define user_online          3
    char buf[_DEF_SQLIEN] = "";
    sprintf(buf, "select password ,id from t_UserData where name = '%s';", rq->m_szUser);
    list<string> resList;
    bool res = m_sql->SelectMysql(buf , 2 , resList);
    if( !res )
    {
        cout<< "SelectMysql errot" << buf <<endl;
    }
    if( resList.size() > 0 )
    {

        if(  strcmp( resList.front().c_str()  ,rq->m_szPassword ) == 0  )
        {
            rs.m_lResult = login_sucess;
            resList.pop_front();
            rs.m_UserID = atoi(resList.front().c_str());

            //存储映射关系
            this->m_mapIDToUserFD[rs.m_UserID ] = clientfd;

//            m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );

//            //用户获取列表

//            return;
        }else
        {
            rs.m_lResult = password_error;
        }
    }else
    {
        rs.m_lResult = userid_no_exist;
    }

    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );

}
//上传请求
void TcpKernel::UploadRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d UploadRq\n", clientfd);

    STRU_UPLOAD_RQ * rq = (STRU_UPLOAD_RQ *)szbuf;

    FileInfo * info = new FileInfo;
    info->m_nPos = 0;
    memcpy( info->m_Hobby , rq->m_szHobby,DEF_HOBBY_COUNT)  ;

    info->m_nUserId = rq->m_UserId;
    info->m_nFileID = rq->m_nFileId;
    info->m_VideoID = 0;

    info->m_nFileSize=rq->m_nFileSize;

    strcpy( info->m_szFileName , rq->m_szFileName );

    strcpy( info->m_szFileType   , rq->m_szFileType );
    //找到用户名

    char sqlstr[_DEF_SQLIEN ] ="";
    sprintf(sqlstr , "select name from t_UserData where id = %d;" ,  info->m_nUserId );
    list<string> resList;
    if( !m_sql->SelectMysql(sqlstr, 1, resList) )
    {
        cout<< "SelectMysql errot" << sqlstr <<endl;
        delete info;
        return;
    }
    if( resList .size() <= 0 )
    {
        delete info;
        return;
    }

    strcpy ( info->m_UserName   ,resList.front().c_str() );
    sprintf( info->m_szFilePath , "%sflv/%s/%s"  , RootPath , info->m_UserName , info->m_szFileName) ;
    sprintf ( info->m_szRtmp , "//%s/%s" ,info->m_UserName , info->m_szFileName );
    if( strcmp(rq->m_szFileType , "gif" ) != 0 )
    {
        strcpy( info->m_szGifName   , rq->m_szGifName );
        sprintf( info->m_szGifPath   , "%sflv/%s/%s" , RootPath , info->m_UserName , info->m_szGifName);
    }
    info->pFile = fopen( info->m_szFilePath , "w");

    m_mapFileIDToFileInfo[ info->m_nFileID ] =  info;
}


//上传文件块
void TcpKernel::UploadBlockRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d UploadBlockRq\n", clientfd);

    STRU_FILEBLOCK_RQ *rq =(STRU_FILEBLOCK_RQ *)szbuf;
    if( m_mapFileIDToFileInfo .find ( rq->m_nFileId ) == m_mapFileIDToFileInfo.end() ) return;

    FileInfo* info  = m_mapFileIDToFileInfo [ rq->m_nFileId ];

    int64_t res = fwrite( rq->m_szFileContent , 1 , rq->m_nBlockLen , info->pFile );
    info->m_nPos += res;


    if(  rq->m_nBlockLen < MAX_CONTENT_LEN || info->m_nPos >= info->m_nFileSize  )
    {
        //写完了
        fclose( info->pFile);
        //判断 不是gif 写表记录   返回信息
        if(  strcmp( info ->m_szFileType ,"gif") != 0)
        {
            //xiebiao
//create table t_VideoInfo( videoid bigint unsigned  AUTO_INCREMENT primary key ,
//                          userId bigint unsigned , videoName nvarchar (300),
//                          picName nvarchar (300), videoPath nvarchar (300) ,
//                          picPath nvarchar (300) ,rtmp nvarchar (300) , food int, funny int ,ennegy int ,dance int ,
//                          music int,  video int,  outside int, edu int , hotdegree int);

            char sqlstr[_DEF_SQLIEN] = "";
            sprintf( sqlstr , "insert into t_VideoInfo ( userId , videoName, picName ,videoPath ,picPath ,rtmp ,food ,funny  ,ennegy  ,dance , music ,  video ,  outside , edu  , hotdegree) values (%d , '%s' ,'%s', '%s' , '%s' , '%s' , %d ,%d , %d ,%d , %d , %d , %d ,%d , %d);"
                     ,rq->m_nUserId , info->m_szFileName , info->m_szGifName, info->m_szFilePath , info->m_szGifPath , info->m_szRtmp
                     , info->m_Hobby[0] ,info->m_Hobby[1],info->m_Hobby[2],info->m_Hobby[3]
                    ,info->m_Hobby[4],info->m_Hobby[5],info->m_Hobby[6],info->m_Hobby[7] , 0 );

            printf("%s\n" , sqlstr);
            if( !m_sql->UpdataMysql(sqlstr ) )
            {
                cout<< "UpdataMysql erro :" << sqlstr <<endl;
            }
            //返回
            STRU_UPLOAD_RS rs;
            rs.m_nResult = 1;
            m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs));
        }

        m_mapFileIDToFileInfo.erase( rq->m_nFileId );
        delete info;
        info = NULL;
    }
}

//处理客户端下载
void TcpKernel::DownloadRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d DownloadRq\n", clientfd);
    //给这个用户推荐   发送动图 和 视频对应的rtmp地址

    STRU_DOWNLOAD_RQ * rq = (STRU_DOWNLOAD_RQ * )szbuf;
    list<FileInfo*> fileList;

    GetFileList(fileList ,rq->m_nUserId);

    //根据list 发文件头 和文件内容
    while( fileList.size() > 0 )
    {
        FileInfo* info = fileList.front();
        fileList.pop_front();

        STRU_DOWNLOAD_RS rs;
        strcpy( rs.m_rtmp , info->m_szRtmp );
        rs.m_nFileId = info->m_nFileID;
        rs.m_nVideoId = info->m_VideoID;
        rs.m_nFileSize = info->m_nFileSize;
        strcpy( rs.m_szFileName , info->m_szFileName );

        m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs));

        info->pFile = fopen( info->m_szFilePath , "r");
        if( info->pFile )
        {
            while(1)
            {
                STRU_FILEBLOCK_RQ blockrq;
                int64_t res = fread( blockrq.m_szFileContent, 1, MAX_CONTENT_LEN /*, 1*/ , info->pFile );
                blockrq.m_nBlockLen = res;
                info->m_nPos += res;
                blockrq.m_nFileId = info->m_VideoID;
                blockrq.m_nUserId = rq->m_nUserId;

                m_tcp->SendData( clientfd , (char*)&blockrq , sizeof(blockrq) );

                 if( info->m_nPos >= info->m_nFileSize )
                 {
                     fclose(info->pFile);
                     delete info;
                     info = NULL;
                     break;
                 }
            }
        }
    }
}
//获取列表
void TcpKernel::GetFileList( list<FileInfo*>& fileList  , int userid )
{

    // 1. 根据userid 找用户没有接受过 in (t_UserRecv --> videoid )的视频的个数
//    select count(videoId) from t_VideoInfo where t_VideoInfo.videoId not in ( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = 5 ); 可以作为推荐的视频个数
//    如果返回的数量是 = 0  , 那么清空 t_UserRecv里的userId的数据
//    delete from t_UserRecv  where  userId = 5;
    char sqlStr[1024] ="";
    sprintf(sqlStr , "select count(videoId) from t_VideoInfo where t_VideoInfo.videoId not in ( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = %d );", userid);
    int nCount = 0;
    list<string> resList;
    if( !m_sql->SelectMysql( sqlStr , 1 , resList))
    {
        printf("SelectMysql error : %s\n" , sqlStr);
        return;
    }
    if( resList.size() == 0 ) return;
    nCount = atoi( resList.front().c_str() );
    // 2. 个数 == 0 根据userid 清空 t_UserRecv 中
    if( nCount == 0 )
    {
        sprintf(sqlStr , "delete from t_UserRecv  where  userId = %d;", userid);
        if( !m_sql->UpdataMysql(  sqlStr  ) )
        {
            printf("UpdataMysql error : %s\n" , sqlStr);
            return;
        }
    }
    // 上面解决一定有视频可以推送
    //3. 推送 用户没有接受过的 热度 hotdegree 前10 order by hotdegree limit 0, 10 的视频 写入--> fileList
    // select videoId ,picName , picPath , rtmp , hotdegree from t_VideoInfo where t_VideoInfo.videoId not in ( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = 5 ) order by hotdegree desc limit 0,10;
    resList.clear();
    sprintf(sqlStr , "select videoId ,picName , picPath , rtmp  from t_VideoInfo where t_VideoInfo.videoId not in ( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = %d ) order by hotdegree desc limit 0,10;", userid);
    if( !m_sql->SelectMysql( sqlStr , 4 , resList))
    {
        printf("SelectMysql error : %s\n" , sqlStr);
        return;
    }
    nCount = 1;
    int nSize = resList.size()/4 ;
    for( int i = 0 ; i < nSize ; ++i )
    {
        FileInfo * info = new FileInfo;
        //info 赋值

        info->m_nPos = 0;

        info->m_VideoID = atoi( resList.front().c_str() );
        resList.pop_front();

        strcpy( info->m_szFileName  ,  resList.front().c_str() );
        resList.pop_front();

        strcpy( info->m_szFilePath ,  resList.front().c_str() );
        resList.pop_front();

        strcpy(info->m_szRtmp , resList.front().c_str() );
        resList.pop_front();

        info->m_nFileID = nCount++;
//        info->m_nUserId;
//        info->m_UserName;
        info->pFile = fopen( info->m_szFilePath , "r");
        fseek( info->pFile , 0, SEEK_END);

        info->m_nFileSize  = ftell(info->pFile);
        fseek( info->pFile , 0, SEEK_SET);
        fclose(info->pFile);
//        info->m_szGifName;
//        info->m_szGifPath;
        info->pFile = NULL;
        //list 添加
        fileList.push_back(info);
        //4. t_UserRecv中插入信息

//        插入已推送数据
//        insert into t_UserRecv values(%d ,%d);
        sprintf(sqlStr , "insert into t_UserRecv values(%d ,%d);", userid , info->m_VideoID  );
        if( !m_sql->UpdataMysql( sqlStr ))
        {
            printf("UpdataMysql error : %s\n" , sqlStr);
            return;
        }
    }
}
