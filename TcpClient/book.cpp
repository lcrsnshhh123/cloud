#include "book.h"
#include "tcpclient.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"
#include "showfile.h"

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    strPwd = TcpClient::getInstance().getCurPath();
    qDebug()<<"构造函数pwd："<<strPwd;
    strName = TcpClient::getInstance().getLoginName();
    m_bDownload = false;
    m_pTimer = new QTimer;

    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名文件夹");
    m_pFlushFilePB = new QPushButton("刷新文件");
    //垂直布局
    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件");
    m_pDownloadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pSharePB = new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);
    //常规文件垂直布局
    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownloadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pSharePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    //总布局水平布局
    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);
    //this.set 设置总布局
    setLayout(pMain);
    //删除要包含目录信息和要删除的文件名
    connect(m_pCreateDirPB,SIGNAL(clicked(bool)),this,SLOT(createDir()));
    connect(m_pFlushFilePB,SIGNAL(clicked(bool)),this,SLOT(flushFile()));
    //双击
    connect(m_pBookListW,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(showDirFile(QModelIndex)));
    //返回按钮
    connect(m_pReturnPB,SIGNAL(clicked(bool)),this,SLOT(returnDirFile()));
    //删除文件夹
    connect(m_pDelDirPB,SIGNAL(clicked(bool)),this,SLOT(delDir()));
    connect(m_pRenamePB,SIGNAL(clicked(bool)),this,SLOT(renameFile()));
    //上传文件
    connect(m_pUploadPB,SIGNAL(clicked(bool)),this,SLOT(uploadFile()));
    //timeout信号  时间到了
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(uploadFileData()));
    connect(m_pDelFilePB,SIGNAL(clicked(bool)),this,SLOT(delRegFile()));
    connect(m_pDownloadPB,SIGNAL(clicked(bool)),this,SLOT(downloadFile()));
    //分享按钮
    connect(m_pSharePB,SIGNAL(clicked(bool)),this,SLOT(shareFile()));
    //移动目录
    connect(m_pMoveFilePB,SIGNAL(clicked(bool)),this,SLOT(moveFile()));
    connect(m_pSelectDirPB,SIGNAL(clicked(bool)),this,SLOT(selectDestDir()));
}
/**
 * 更新listWidge内容
 * @brief Book::updateFileList
 * @param pdu
 */
void Book::updateFileList(const PDU *pdu)
{
    if(NULL == pdu)
        return;

    QListWidgetItem *pItemTmp = NULL;//图标和文件信息
    int row = m_pBookListW->count();//ListWidget行数
    while(m_pBookListW->count()>0)
    {
        pItemTmp = m_pBookListW->item(row-1);
        m_pBookListW->removeItemWidget(pItemTmp);//删除item
        delete pItemTmp;//释放内存
        row -=1;
    }
    FileInfo *pFileInfo = NULL;
    int count = pdu->uiMsgLen/sizeof(FileInfo);
    for(int i=0;i<count;i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(0 == pFileInfo->iFileType)
            pItem->setIcon(QIcon(QPixmap(":/DIR.jpg")));//设置图片
        else if(1 == pFileInfo->iFileType)
            pItem->setIcon(QIcon(QPixmap(":/FILE.jpg")));
        pItem->setText(pFileInfo->caFileName);//设置文本
        m_pBookListW->addItem(pItem);//添加一行
    }
}

/**
 * 设置下载状态
 * @brief Book::setDownloadStatus
 * @param status
 */
void Book::setDownloadStatus(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_shareFileName;
}

QString Book::getStrPwd()
{
    return strPwd;
}

/**
 * 创建文件夹操作
 * @brief Book::createDir
 */
void Book::createDir()
{
    //QInputDialog 专门获得输入
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","新文件夹名字");
    if(!strNewDir.isEmpty())
    {
        if(strNewDir.size()>32)
            QMessageBox::warning(this,"新建文件夹","文件夹名超出最大长度");
        else
        {
            QString strName = TcpClient::getInstance().getLoginName();
            qDebug()<<strPwd;
            QString strCurPath = TcpClient::getInstance().getCurPath();
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());
            strncpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());
            memcpy(pdu->caMsg,strPwd.toStdString().c_str(),strPwd.size());

            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
    else
    {
        QMessageBox::warning(this,"新建文件夹","文件夹名不为空!");
    }
}

/**
 * 刷新显示文件夹
 * @brief Book::flushFile
 */
void Book::flushFile() //查看文件按钮槽函数
{
    //当前目录
    strPwd = TcpClient::getInstance().getCurPath();
    PDU *pdu = mkPDU(strPwd.size()+1);
    qDebug()<<"flush strPwd:"<<strPwd;
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
    strncpy((char*)(pdu->caMsg),strPwd.toStdString().c_str(),strPwd.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

/**
 * 双击按钮进入下一层目录
 * @brief Book::showDirFile
 * @param item
 */
void Book::showDirFile(QModelIndex item)
{
    //item.data()获得双击的内容
    if(item.data().toString()== "..")
        return;
    else
    {
        qDebug()<<item.data().toString();
        //当前目录
        strPwd += "/"+item.data().toString();
        qDebug()<<strPwd;
        PDU *pdu = mkPDU(strPwd.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
        strncpy((char*)(pdu->caMsg),strPwd.toStdString().c_str(),strPwd.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

/**
 * 删除文件夹
 * @brief Book::delDir
 */
void Book::delDir()
{
    //QListWidgetItem 行item
    QListWidgetItem *pItem = m_pBookListW->currentItem();//选择的行
    if(NULL == pItem)
        QMessageBox::warning(this,"删除","请选择要删除的文件夹");
    else
    {
        QString strDelName = pItem->text();//获取选择的文本
        PDU *pdu = mkPDU(strPwd.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy((char*)(pdu->caMsg),strPwd.toStdString().c_str(),strPwd.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

/**
 * 返回按钮
 * @brief Book::returnDirFile
 */
void Book::returnDirFile()
{
    QString temPwd = strPwd;//strpwd当前目录
    strPwd += "/..";
    qDebug()<<strPwd;
    PDU *pdu = mkPDU(strPwd.size()+1);
    QString strPath = QString("./%1/..").arg(strName);
    if(strPwd == strPath)//顶层目录
    {
        free(pdu);
        pdu = NULL;
        QMessageBox::information(this,"返回","当前目录不能再返回");
        strPwd = QString("./%1").arg(strName);
        return;
    }
    else
    {
        //"./aa/bb/cc  ./aa/bb"
        QStringList strList = temPwd.split("/");
        strList.pop_back();
        strPwd = strList.join("/");//这一行将经过上述处理的 strList 中的元素用 / 分隔符连接起来，
        qDebug()<<strPwd;

        pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
        strName = TcpClient::getInstance().getLoginName();
        memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
        strncpy((char*)(pdu->caMsg),strPwd.toStdString().c_str(),strPwd.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }

}

/**
 * 重命名
 * @brief Book::renameFile
 */
void Book::renameFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
        QMessageBox::warning(this,"重命名","请选择要重命名的文件");
    else
    {
        QString strOldName = pItem->text();
        //输入对话框
        QString strNewName = QInputDialog::getText(this,"重命名文件","请输入新的文件名:");
        if(!strNewName.isEmpty())
        {
            PDU *pdu = mkPDU(strPwd.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            //char caData[64
            strncpy(pdu->caData,strOldName.toStdString().c_str(),strOldName.size());
            strncpy(pdu->caData+32,strNewName.toStdString().c_str(),strNewName.size());
            memcpy((char*)(pdu->caMsg),strPwd.toStdString().c_str(),strPwd.size());

            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this,"重命名文件","新文件名不为空");
        }
    }

}

/**
 * 上传文件槽函数
 * @brief Book::uploadFile
 */
void Book::uploadFile()
{
    uploadFilePath = QFileDialog::getOpenFileName();//文件对话框
    qDebug()<<uploadFilePath;

    if(!uploadFilePath.isEmpty())
    {
        //获取上传文件名
        int index = uploadFilePath.lastIndexOf('/');
        //aa/bb/cc 5 8
        //从右边数的长度
        QString strFileName = uploadFilePath.right(uploadFilePath.size()-index-1);
        qDebug()<<strFileName;

        QFile file(uploadFilePath);
        qint64 fileSize = file.size();  //获取文件大小

        PDU *pdu = mkPDU(uploadFilePath.size()+1);//文件名
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg,strPwd.toStdString().c_str(),strPwd.size());
        sprintf(pdu->caData,"%s %lld",strFileName.toStdString().c_str(),fileSize);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
        //刚请求完不能直接上传二进制文件

        m_pTimer->start(500);//定时器
    }
    else
    {
        QMessageBox::warning(this,"上传文件","上传文件不为空");
    }
}

/**
 * 再次发送上传请求  定时器到达发送文件
 * @brief Book::uploadFileData
 */
void Book::uploadFileData()
{
    m_pTimer->stop();//关掉定时器
    QFile file(uploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"上传文件","打开文件失败");
        return;
    }
    char *pBuffer = new char[4096];//4096效率较高
    qint64 ret = 0;
    while(true)
    {
        ret = file.read(pBuffer,4096);
        if(ret>0&&ret<=4096)
        {
            //这是因为write函数只是将文件数据写在了发送端的缓冲区上，并没有进行发送，如果不加限制，会一直往本地缓冲区写数据，导致内存溢出，解决方法是控制socket及时发送数据，我们可以设定一个门限值，用bytesToWrite()获取缓冲区中待发送的字节数，
            //当超过门限值，使用flush()函数控制socket将缓冲区数据发送出去，这样就可以避免内存溢出问题了。
            TcpClient::getInstance().getTcpSocket().write(pBuffer,ret);
            if(TcpClient::getInstance().getTcpSocket().bytesToWrite()>10000)
            {
                TcpClient::getInstance().getTcpSocket().flush();
            }
        }
        else if(ret == 0)
        {
            break;
        }
        else
        {
            QMessageBox::warning(this,"上传文件","上传文件失败:读文件失败");
            break;
        }
    }
    file.close();
    delete[] pBuffer;
    pBuffer = NULL;
}

/**
 * 删除常规文件  包含目录信息和要删除的文件名
 * @brief Book::delRegFile
 */
void Book::delRegFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
        QMessageBox::warning(this,"删除","请选择要删除的文件");
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strPwd.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy((char*)(pdu->caMsg),strPwd.toStdString().c_str(),strPwd.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

/**
 * 下载文件  发送当前所在网盘路径和下载文件名
 * @brief Book::downloadFile
 */
void Book::downloadFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    QString strFileName = pItem->text();
    if(NULL == pItem)
        QMessageBox::warning(this,"下载","请选择要下载的文件");
    else
    {
        QString strSaveFilePath = QFileDialog::getSaveFileName();
        if(strSaveFilePath.isEmpty())
        {
            m_strSaveFilePath.clear();
            QMessageBox::warning(this,"下载文件","下载路径不为空");
        }
        else
        {
            //本地的下载路径
            m_strSaveFilePath = strSaveFilePath;//下载路径
            //            m_bDownload = true;
        }
        PDU *pdu = mkPDU(strPwd.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        strcpy(pdu->caData,strFileName.toStdString().c_str());
        //服务器当前路径
        memcpy((char*)(pdu->caMsg),strPwd.toStdString().c_str(),strPwd.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

/**
 * 分享文件  //
 * @brief Book::shareFile
 */
void Book::shareFile()
{
    //获得要分享的文件
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"分享","请选择要分享的文件");
        return;
    }
    else
    {
        m_shareFileName = pItem->text();//获得分享文件名
    }
    Friend *pFriend = OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList = pFriend->getFriendList();//获得链表
    ShowFile::getInstance().updateFriend(pFriendList);
    if(ShowFile::getInstance().isHidden())
        ShowFile::getInstance().show();
}

void Book::moveFile()
{
    QListWidgetItem * pCurItem = m_pBookListW->currentItem();
    if(NULL!=pCurItem)
    {
        m_strMoveFileName = pCurItem->text();
        m_moveFilePath = strPwd+'/'+ m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);//使能文件夹
    }
    else
    {
        QMessageBox::warning(this,"移动文件","请选择要移动的文件");
    }
}

void Book::selectDestDir()
{
    QListWidgetItem * pCurItem = m_pBookListW->currentItem();
    if(NULL!=pCurItem)
    {
        QString strDestDir = pCurItem->text();
        m_strDestDir = strPwd+'/'+ strDestDir;

        int srcLen = m_moveFilePath.size();
        int destLen = m_strDestDir.size();
        PDU *pdu = mkPDU(srcLen+destLen+2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str());

        memcpy(pdu->caMsg,m_moveFilePath.toStdString().c_str(),srcLen);
        //前面有\0
        memcpy((char*)(pdu->caMsg)+srcLen+1,m_strDestDir.toStdString().c_str(),destLen);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"移动文件","请选择要移动的文件");
    }
    m_pSelectDirPB->setEnabled(false);
}

