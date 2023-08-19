#ifndef SHOWFILE_H
#define SHOWFILE_H


#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>//进度条
#include <QListWidget>

//点击
class ShowFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShowFile(QWidget *parent = nullptr);

    static ShowFile &getInstance();

    void updateFriend(QListWidget *pFriendList);

signals:
private slots:
    void cancelSelect();
    void selectAll();
    void okShare();
    void cancelShare();

private:
    QPushButton *m_pSelectAllPB;//全选按钮
    QPushButton *m_pCancelSelectPB;//反选按钮

    QPushButton *m_pOKPB;//确定按钮
    QPushButton *m_CanclePB;//取消按钮

    QScrollArea *m_pSA;//显示好友区域 滑滚区
    QWidget *m_pFriendW;//QWidget 放到QScrollArea中
    QButtonGroup *m_pButtonGroup;

    QVBoxLayout *m_pFriendWVBL;//垂直布局

};

#endif // SHOWFILE_H
