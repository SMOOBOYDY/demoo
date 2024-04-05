#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H



#include "TCPNet.h"
#include "Mysql.h"
#include<map>

class TcpKernel;
typedef void (TcpKernel::*PFUN)(int,char*,int nlen);

typedef struct
{
    PackType m_type;
    PFUN m_pfun;
} ProtocolMap;



class TcpKernel:public IKernel
{
public:
    int Open();
    void Close();
    void DealData(int,char*,int);

    //注册
    void RegisterRq(int,char*,int);
    //登录
    void LoginRq(int,char*,int);
    //上传请求
    void UploadRq(int,char*,int);
    //上传文件块
    void UploadBlockRq(int,char*,int);
    //下载
    void DownloadRq(int,char*,int);
    //获取推荐列表
    void GetFileList(list<FileInfo *> &fileList, int userid);
private:
    CMysql * m_sql;
    TcpNet * m_tcp;

//    map<int , UserInfo*> m_mapIDToUserInfo;
    map<int , int> m_mapIDToUserFD;
//    map<int , list<UserInfo*> > m_mapRoomIDToUserList;
    map< int , FileInfo* > m_mapFileIDToFileInfo;

};

#endif
