#include "privatechat.h"
#include "ui_privatechat.h"
#include <string.h>
#include <QMessageBox>
#include"tcpclient.h"
PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    this->setWindowTitle("私聊");

connect(ui->lineEdit_inputMsg,SIGNAL(returnPressed()),ui->pushButton_sendMsg,
            SIGNAL(clicked()));//关联俩个信号


}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().getLoginName();
}

/**
 * 更新消息
 * @brief PrivateChat::updateMsg
 * @param pdu
 */
void PrivateChat::updateMsg(const PDU *pdu)
{
    if(NULL == pdu)
        return;
    char caSendName[32] = {'\0'};
    memcpy(caSendName,pdu->caData,32);
    QString strMsg = QString("%1 says: %2").arg(caSendName).arg((char*)(pdu->caMsg));
    ui->textEdit_showMsg->append(strMsg);//对话框添加数据
}

/**
 * 发送消息
 * @brief PrivateChat::on_pushButton_sendMsg_clicked
 */
void PrivateChat::on_pushButton_sendMsg_clicked()
{
    //获取输入框内容
    QString strMsg = ui->lineEdit_inputMsg->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData,m_strLoginName.toStdString().c_str(),m_strLoginName.size());
        memcpy(pdu->caData+32,m_strChatName.toStdString().c_str(),m_strChatName.size());

        strcpy((char*)pdu->caMsg,strMsg.toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
        ui->lineEdit_inputMsg->clear();//清空行编辑框
    }
    else
    {
        QMessageBox::information(this,"私聊","发送消息不为空!");
    }
}

