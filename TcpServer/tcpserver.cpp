
#include "tcpserver.h"
#include "ui_tcpserver.h"

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
    //解决一个小bug 服务器异常崩溃时 将数据库异常状态都置为0
    QString dataStr = QString(" update userInfo set online=0");

    QSqlQuery qs;
    qDebug()<< qs.exec(dataStr);

}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly)){
        QByteArray data = file.readAll();
        QString strData = data.toStdString().c_str();

        strData.replace("\r\n"," ");
        qDebug()<<strData;

        QStringList strList = strData.split(" ");  //分割字符串

        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUInt();
        qDebug()<<"IP:"<<m_strIP<<"\t"<<"端口:"<<m_usPort;
    }
    else{
        QMessageBox::critical(this,"open config","open error",QMessageBox::Ok);
    }
    file.close();
}


