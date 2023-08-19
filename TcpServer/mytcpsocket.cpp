#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QFileInfoList>
#include <QFileInfo>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    m_bUpload = false;
    m_pTimer = new QTimer;

    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(sendFileToClient()));
    connect(this,&QTcpSocket::readyRead,this,&MyTcpSocket::recvMsg);
    connect(this,&QTcpSocket::disconnected,this,&MyTcpSocket::clientOffline);
}

QString MyTcpSocket::getName()
{
    return m_strName;
}
//拷贝文件夹
void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);
    //设置路径
    dir.setPath(strSrcDir);
    //获得目录其他信息
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString stcTmp;
    QString destTmp;
    for(int i=0;fileInfoList.size();i++)
    {
        qDebug()<<"filename:"<<fileInfoList[i];
        if(fileInfoList[i].isFile())
        {
            stcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            QFile::copy(stcTmp,destTmp);//执行拷贝
        }
        else if(fileInfoList[i].isDir())
        {
            //跳过当前目录和上一级目录
            if(fileInfoList[i].fileName()=="."||fileInfoList[i].fileName()=="..")
                continue;
            stcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+fileInfoList[i].fileName();
            // 子文件夹递归拷贝
            copyDir(stcTmp,destTmp);
        }
    }
}

/**
 * 服务器接收数据
 * @brief MyTcpSocket::recvMsg
 */
void MyTcpSocket::recvMsg()
{
    if(!m_bUpload)
    {
        qDebug()<<this->bytesAvailable();
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen,sizeof(uint));  //获取消息总大小
        uint uiMsgLen = uiPDULen-sizeof(PDU);       //实际消息长度
        PDU *pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));  //接收剩余PDU长度
        qDebug()<<pdu->uiMsgType<<(char*)pdu->caMsg;

        //判断信息类型
        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            //将用户名密码存入PDU的caData里面
            strncpy(caName,pdu->caData,32);
            strncpy(caPwd,pdu->caData+32,32);
            //是否注册成功
            bool ret = OperateDB::getInstance().handleRegister(caName,caPwd);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData,REGIST_OK);
                QDir dir;//类
                qDebug()<<dir.mkdir(QString("./%1").arg(caName));
            }
            else
                strcpy(respdu->caData,REGIST_FAILED);
            write((char*)respdu,respdu->uiPDULen); //发送内容,返回给客户端
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            strncpy(caPwd,pdu->caData+32,32);

            bool ret = OperateDB::getInstance().handleLogin(caName,caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData,LOGIN_OK);
                m_strName = caName;             //登陆成功后保存名字，名字唯一，方便后续修改登陆状态
            }
            else
                strcpy(respdu->caData,LOGIN_FAILED);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            //接收所有在线用户的用户名
            QStringList ret = OperateDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i=0;i<ret.size();i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).toStdString().c_str(),ret.at(i).size());
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            int ret = OperateDB::getInstance().handleSearchUser(pdu->caData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if(3 == ret)
                strcpy(respdu->caData,SEARCH_USER_NO);
            else if (1 == ret)
                strcpy(respdu->caData,SEARCH_USER_ONLINE);
            else if(2 == ret)
                strcpy(respdu->caData,SEARCH_USER_OFFLINE);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        //加好友请求
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);

            int ret = OperateDB::getInstance().handleAddFriend(caPerName,caName);
            PDU *respdu = NULL;
            if(-1 == ret)
            {
                //未知错误
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,UNKNOW_ERROR);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(0 == ret)
            {
                //用户已经是好友
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,EXISTED_FRIEND);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(1 == ret)
            {
                //在线可添加
                MyTcpServer::getInstance().resend(caPerName,pdu);
            }
            else if(2 == ret)
            {
                //用户不在线
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(3 == ret)
            {
                //用户不存在
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_NO_EXIST);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            //涉及数据库操作
            char caName[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList ret = OperateDB::getInstance().handleFlushFriend(caName);
            uint uiMsgLen = ret.size()*32;//数据大小
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for(int i=0;i<ret.size();i++)
            {
                //int * +1偏移4个字节
                memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).toStdString().c_str(),ret.at(i).size());
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_AGREE:
        {
            char caName[32] = {'\0'};
            char caPerName[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            strncpy(caPerName,pdu->caData+32,32);

            OperateDB::getInstance().handleAddFriendSuccess(caName,caPerName);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caSelfName[32] = {'\0'};
            char caFriendName[32] = {'\0'};
            strncpy(caSelfName,pdu->caData,32);
            strncpy(caFriendName,pdu->caData+32,32);
            OperateDB::getInstance().handleDelFriend(caSelfName,caFriendName);

            PDU *respdu = mkPDU(0);
            //回送给删除发起删除好有的人
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy(respdu->caData,DEL_FRIEND_OK);
            this->write((char*)respdu,pdu->uiPDULen);
            //发送您被删除信息  pdu 信息类别是删除请求.
            MyTcpServer::getInstance().resend(caFriendName,pdu);

            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            //对方名字
            char caPerName[32] = {'\0'};
            memcpy(caPerName,pdu->caData+32,32);
            qDebug()<<caPerName;
            MyTcpServer::getInstance().resend(caPerName,pdu);

            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList onlineFriend = OperateDB::getInstance().handleFlushFriend(caName);
            QString tempName;
            for(int i=0;i<onlineFriend.size();i++)
            {
                tempName = onlineFriend.at(i);
                MyTcpServer::getInstance().resend(tempName.toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));
            bool ret = dir.exists(strCurPath);//路径是否存在
            qDebug()<<strCurPath;
            PDU *respdu = NULL;
            if(ret) //当前目录存在
            {
                char caNewDir[32] = {'\0'};
                memcpy(caNewDir,pdu->caData+32,32);
                QString strNewPath = strCurPath+"/"+caNewDir;//拼接目录
                qDebug()<<strNewPath;
                ret = dir.exists(strNewPath);
                qDebug()<<"-->"<<ret;
                if(ret) //创建的文件名已存在
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData,FILE_NAME_EXIST);
                }
                else//创建成功
                {
                    dir.mkdir(strNewPath);
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData,CREATE_DIR_OK);
                }
            }
            else
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData,DIR_NO_EXIST);
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen);
            QDir dir(pCurPath);//
            qDebug()<<pCurPath;
            QFileInfoList fileInfoList = dir.entryInfoList();//获得当前目录下文件信息
            int iFileCount = fileInfoList.size();

            PDU* respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;//自定义结构体
            QString strTempFileName;
            for(int i=0;i<iFileCount;i++)
            {
                //+i一次移动结构体大小的地址
                pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                strTempFileName = fileInfoList[i].fileName();

                memcpy(pFileInfo->caFileName,strTempFileName.toStdString().c_str(),strTempFileName.size());
                if(fileInfoList[i].isDir())
                    pFileInfo->iFileType = 0; //文件夹类型
                else if(fileInfoList[i].isFile())
                    pFileInfo->iFileType = 1;
                qDebug()<<fileInfoList[i].fileName()
                         <<fileInfoList[i].size()
                         <<"文件夹:"<<fileInfoList[i].isDir()
                         <<"常规文件:"<<fileInfoList[i].isFile();
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if(fileInfo.isDir())//是否是路径
            {
                QDir dir;
                dir.setPath(strPath);
                ret = dir.removeRecursively();//删除目录和子目录
                qDebug()<< ret;
            }
            else if(fileInfo.isFile())
            {
                ret = false;
                qDebug()<< ret;
            }
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(strlen(DEL_DIR_OK)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
            }
            else
            {
                respdu = mkPDU(strlen(DEL_DIR_FAILURED)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_FAILURED,strlen(DEL_DIR_FAILURED));
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            pdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            char caOldName[32] = {'\0'};
            char caNewName[32] = {'\0'};
            strncpy(caOldName,pdu->caData,32);
            strncpy(caNewName,pdu->caData+32,32);

            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);

            qDebug()<<strOldPath;
            qDebug()<<strNewPath;
            delete pPath;
            QDir dir;//创建了一个默认构造的 QDir 对象，表示当前工作目录。
            bool ret = dir.rename(strOldPath,strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData,RENAME_FILE_OK);
            }
            else
            {
                strcpy(respdu->caData,RENAME_FILE_FAILURED);
            }

            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            pdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caEnterName[32] = {'\0'};
            strncpy(caEnterName, pdu->caData, 32);

            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

            QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);

            qDebug() << strPath;

            QFileInfo fileInfo(strPath);
            PDU *respdu = NULL;
            if (fileInfo.isDir())
            {
                QDir dir(strPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                respdu = mkPDU(sizeof(FileInfo)*iFileCount);
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;//与刷新操作相似
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                for (int i=0; i<iFileCount; i++)
                {
                    pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                    strFileName = fileInfoList[i].fileName();

                    memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                    if (fileInfoList[i].isDir())
                    {
                        pFileInfo->iFileType = 0;
                    }
                    else if (fileInfoList[i].isFile())
                    {
                        pFileInfo->iFileType = 1;
                    }
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if (fileInfo.isFile())
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData, ENTER_DIR_FAILURED);

                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }

            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            qint64 fileSize = 0;
            //取出数据
            sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);
            char *pPath = new char[pdu->uiMsgLen];//信息大小
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            qDebug()<<strPath;
            delete []pPath;//释放开辟的空间
            pPath = NULL;
            //要上传的文件路径
            m_file.setFileName(strPath);
            // 以只写的方式打开文件，若文件不存在则自动创建文件
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if(fileInfo.isDir())
            {
                ret = false;
            }
            else if(fileInfo.isFile())
            {
                QDir dir;//默认当前目录
                ret = dir.remove(strPath);
                qDebug()<<ret;
            }
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(strlen(DEL_FILE_OK)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK));
            }
            else//删除失败
            {
                respdu = mkPDU(strlen(DEL_FILE_FAILURED)+1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_FAILURED,strlen(DEL_FILE_FAILURED));
            }

            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            pdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            strcpy(caFileName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,(char*)(pdu->caMsg),pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            qDebug()<<strPath;
            delete []pPath;
            pPath = NULL;

            qDebug()<<strPath;
            m_file.setFileName(strPath);

            m_file.open(QIODevice::ReadOnly);
            bool ret = m_file.isOpen();
            qDebug()<<ret;

            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();//获得文件大小
            PDU *respdu = mkPDU(0);
            sprintf(respdu->caData,"%s %lld",caFileName,fileSize);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;

            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            //打开文件
            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(500);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char caSendName[32] = {'\0'};
            int num = 0;
            //提取信息 num是接受者的个数
            sscanf(pdu->caData,"%s%d",caSendName,&num);
            int size = 32*num;
            //路径所占空间pdu->uiMsgLen-size
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            //信息类型
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caData,caSendName);
            //共享文件路径
            memcpy(respdu->caMsg,(char*)(pdu->caMsg)+size,pdu->uiMsgLen-size);

            char caRecvName[32] = {'\0'};
            for(int i=0;i<num;i++)
            {
                memcpy(caRecvName,(char*)(pdu->caMsg)+i*32,32);
                //根据接受者用户名转发
                MyTcpServer::getInstance().resend(caRecvName,respdu);
            }
            free(respdu);
            respdu = NULL;

            //回复给发送者 ok
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData,"share file ok");
            this->write((char*)respdu,respdu->uiPDULen);

            free(respdu);
            respdu = NULL;
            break;
        }
        //共享文件通知
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {
            QString strRecvPath = QString("./%1").arg(pdu->caData);
            QString strShareFilePath = QString("%1").arg((char*)pdu->caMsg);
            int index = strShareFilePath.lastIndexOf('/');
            //右边文件名字
            QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1);
            //拼接到目标目录后面
            strRecvPath = strRecvPath+"/"+strFileName;
            qDebug()<<"分享路径:"<<strShareFilePath;
            qDebug()<<"保存路径:"<<strRecvPath;

            QFileInfo fileInfo(strShareFilePath);
            if(fileInfo.isFile())
            {
                //拷贝文件
                qDebug()<<"是否复制:"<<QFile::copy(strShareFilePath,strRecvPath);
            }
            else if(fileInfo.isDir())
            {
                //拷贝文件夹
                copyDir(strShareFilePath,strRecvPath);
            }
            break;
        }

        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caData,"%d%d%s",&srcLen,&destLen,caFileName);

            char *pSrcPath = new char[srcLen+1];
            char *pDestPath = new char[destLen+1+32];
            memset(pSrcPath,'\0',srcLen+1);
            memset(pDestPath,'\0',destLen+1+32);

            memcpy(pSrcPath,pdu->caMsg,srcLen);
            memcpy(pDestPath,(char*)(pdu->caMsg)+(srcLen+1),destLen);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            QFileInfo fileInfo(pDestPath);
            if(fileInfo.isDir())
            {
                strcat(pDestPath,"/");
                strcat(pDestPath,caFileName);
                //目标路径+文件名
                bool ret = QFile::rename(pSrcPath,pDestPath);//文件名 移动 路径
                if(ret)
                {
                    strcpy(respdu->caData,MOVE_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData,COMOM_ERROR);
                }
            }
            else if(fileInfo.isFile())
            {
                strcpy(respdu->caData,MOVE_FILE_FAILURED);//移动失败
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = NULL;
    }
    else //是二进制状态 上传文件类型
    {
        PDU *respdu = mkPDU(0);
        QByteArray buff = this->readAll();//接收读到的数据
        m_file.write(buff);//m_file是文件路径+文件名字
        m_iRecved +=buff.size();
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        if(m_iTotal == m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_OK);

            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_iTotal < m_iRecved)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_FAILURED);

//            QByteArray tmpMsg = QString("%1").arg((char*)respdu).toUtf8();
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
    }
}

void MyTcpSocket::clientOffline()
{
    OperateDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this); //发送信号
}

/**
 * 发送文件给客户端
 * @brief MyTcpSocket::sendFileToClient
 */
void MyTcpSocket::sendFileToClient()
{
    char *pData = new char[4096];//new 自动清空
    qint64 ret = 0;
    while(true)
    {
        ret = m_file.read(pData,4096);//读到缓冲区
        if(0<ret && ret>=4096)
        {
            this->write(pData,ret); //发送到tcp缓冲区
        }
        else if(ret < 0)
        {
            qDebug()<<"发送文件内容给客户端过程出错";
            m_pTimer->stop();
            m_file.close();
            break;
        }
        else
        {
            m_pTimer->stop();
            m_file.close();
            break;
        }

    }
    m_file.close();
    delete[] pData;
    pData = NULL;
}
