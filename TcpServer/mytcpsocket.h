
#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QDir>
#include <QFile>
#include <QTimer>

#include "protocol.h"
#include "operatedb.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);

    QString getName();

    void copyDir(QString strSrcDir,QString strDestDir);
signals://信号
    void offline(MyTcpSocket* mysocket);

public slots:
    void recvMsg();
    void clientOffline();   //用户端下线处理函数
    void sendFileToClient();
private:
    QString m_strName;

    QFile m_file;//文件
    qint64 m_iTotal;//总大小
    qint64 m_iRecved;//收到的大小
    bool m_bUpload;//是否是上传文件状态

    QTimer *m_pTimer;
};

#endif // MYTCPSOCKET_H
