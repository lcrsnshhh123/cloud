#include "showfile.h"
#include <QCheckBox>
#include <QList>
#include "tcpclient.h"
#include "opewidget.h"

ShowFile::ShowFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");
    m_pOKPB = new QPushButton("确定");
    m_CanclePB = new QPushButton("取消");

    m_pSA = new QScrollArea();
    m_pFriendW = new QWidget();
    m_pButtonGroup = new QButtonGroup(m_pFriendW);//全选
    //可以指定父窗口，那么它将作为父窗口中布局其它控件的工具
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);//布局
    m_pButtonGroup->setExclusive(false);//可以选择多个

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_CanclePB);
    pDownHBL->addStretch();

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);

    setLayout(pMainVBL);

    connect(m_pCancelSelectPB,SIGNAL(clicked(bool)),this,SLOT(cancelSelect()));
    connect(m_pSelectAllPB,SIGNAL(clicked(bool)),this,SLOT(selectAll()));
    connect(m_pOKPB,SIGNAL(clicked(bool)),this,SLOT(okShare()));
    connect(m_CanclePB,SIGNAL(clicked(bool)),this,SLOT(cancelShare()));
}

ShowFile &ShowFile::getInstance()
{
    static ShowFile instance;
    return instance;
}

void ShowFile::updateFriend(QListWidget *pFriendList)
{
    if(NULL == pFriendList)
        return;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    QAbstractButton *tmp = NULL;
    for(int i=0;i<preFriendList.size();i++)
    {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    QCheckBox *pCB = NULL;
    for(int i=0;i<pFriendList->count();i++)
    {
        pCB = new QCheckBox(pFriendList->item(i)->text());//复选框
        m_pFriendWVBL->addWidget(pCB);//纵向布局添加窗口
        m_pButtonGroup->addButton(pCB);//添加到按钮组
    }
    m_pSA->setWidget(m_pFriendW);//m_pFriendW显示到m_pSA滑滚区上上
}

void ShowFile::cancelSelect()
{
    //获得所有按钮
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i=0;i<cbList.count();i++)
    {
        if(cbList[i]->isChecked())//如果被勾选
            cbList[i]->setChecked(false); //取消勾选
    }
}

void ShowFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i=0;i<cbList.count();i++)
    {
        if(!cbList[i]->isChecked())
            cbList[i]->setChecked(true);
    }
}
//分享者+接受者数字  接受者名字+文件名
void ShowFile::okShare()
{
    //分享者
    QString strName = TcpClient::getInstance().getLoginName();
    //当前网盘路径
    QString strCurPath = OpeWidget::getInstance().getBook()->getStrPwd();
    //要分享的文件名
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();
    //网盘路径+文件名
    QString strPath = strCurPath+"/"+strShareFileName;
    //获得所有指针
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int num = 0;
    int j = 0;
    for(int i=0;i<cbList.count();i++)
    {
        if(cbList[i]->isChecked())//以及被选上
        {
            num++;
        }
    }

    PDU *pdu = mkPDU(32*num+strPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num);
    //32存一个人名
    for(int i=0;i<cbList.count();i++)
    {
        if(cbList[i]->isChecked())
        {
            memcpy((char*)(pdu->caMsg)+j*32,cbList[i]->text().toStdString().c_str(),32);
            j++;
        }
    }
    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(),strPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

void ShowFile::cancelShare()
{
    this->hide();//隐藏当前窗口
}

