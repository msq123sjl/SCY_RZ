#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QTcpSocket>
#include "api/myapp.h"
#include "api/mythread.h"
#include "api/message.h"
#include "frmmain.h"
#include <QMutex>
//延时，TIME_OUT是串口读写的延时
#define TIME_OUT 100

namespace Ui {
class frmMain;
}

class frmMain : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();
    
private:
    Ui::frmMain *ui;
    QLabel *labTime;    //当前时间标签
    QLabel *labWelcom;    //当前时间标签
    QLabel *labUser;    //当前时间标签
    QLabel *labIP;    //当前ip
    QTimer *timerDate;  //计算在线时长和显示当前时间定时器对象
    QTimer *timerSample; //定时读取WEB对采样器的指令
    QTimer *tcpSocketTimer1;
    QTimer *tcpSocketTimer2;
    QTimer *tcpSocketTimer3;
    QTimer *tcpSocketTimer4;
    QTimer *statusTimer;
    QTimer *MinsTimer;
    Message a;
    frmValve v;
    myAPI api;

    //QString columnNames[11];     //列名数组
    int  columnWidths[6];        //列宽数组

    void InitStyle();
    void InitForm();
    void InitTcpSocketClient();
    void InitCOM(int port);
    void InitRtdTable();

protected:
//    void mousePressEvent(QMouseEvent *event);
private slots:
    void ShowDateTime();
//    void MinsPro();
    void displayError(QAbstractSocket::SocketError);
    void clientReadMessage();
    void updateclient();
//    void CheckStatus();
    void ConnectServer();
    bool CheckSampleCmd();
//    void Valve_Status_Check();
    void ShowForm();
    void Status();
    void on_btnClearData_clicked();
    void updateClientStatusDisconnect();

//    void lookedUp(const QHostInfo &host);
//    void on_tableView_clicked(const QModelIndex &index);
};

#endif // FRMMAIN_H
