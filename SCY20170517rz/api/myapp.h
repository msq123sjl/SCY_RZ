#ifndef MYAPP_H
#define MYAPP_H

#include <QString>
#include "../qextserial/qextserialport.h"

typedef struct __COM_PARA
{
    const QString Name;       //串口名称
    int Baudrate;       //串口波特率
    int Databits;       //串口数据位
    int Parity;         //串口校验位
    int Stopbits;       //串口停止位    
//    int Protocol;       //串口协议索引
//    bool use;           //串口是否使用
    int Timeout;        //串口读取超时
    int Interval;       //串口通讯周期
//    int HardwareAddress;//串口硬件地址

}Com_para;

typedef struct __TCP_PARA
{
    bool    ServerOpen;
    QString ServerIP;
    QString ServerPort;
    bool isConnected;

}Tcp_para;

typedef struct __SAMPLER
{
    int SampleMode;                             //采样器型号
    int SampleFlow;                               //采样量
    int SampleStartBottle;                    //开始瓶号

}Sampler;

class myApp
{
public:
    static QString SoftTitle;           //软件标题
    static QString SoftVersion;         //软件版本
    static QString CompanyName;         //开发商名称
    static QString ContactName;         //联系人
    static QString ContactTel;          //联系电话
    static QString LastLoginer;         //最后一次登录用户名,用来自动登录时需要加载的用户信息
    static QString CurrentUserName;     //当前用户名
    static QString CurrentUserPwd;      //当前用户密码
    static QString CurrentUserType;     //当前用户类型（普通用户、管理员）
    static bool Login;                  //系统登录标志
    static int DeskWidth;               //桌面宽度
    static int DeskHeigth;              //桌面高度
    static QString AppPath;             //应用程序路径
    static QString MN;                  //设备编号
    static int  StType;                  //污染源类型
    static int RtdInterval;             //实时数据上传间隔
    static int MinInterval;             //分钟数据上传间隔
    static QString PW;                  //访问密码
    static QString AlarmTarget;         //报警地址
    static QString LocalIP;                  //设备IP
    static QString Mask;                //设备子网掩码
    static QString GateWay;             //设备网关
    static QString DHCP;                   //DHCP服务
    static QString ReportTime;          //日数据上报时间
    static int ReportTimeHour;
    static int ReportTimeMin;
    static int OverTime;                //超时时间
    static int ReCount;                 //超时重发次数
    static int AlarmTime;               //报警时间
    static QString ConfigUart1;         //串口1信息
    static QString ConfigUart2;         //串口2信息
    static QString ConfigUart3;         //串口3信息
    static QString ConfigUart4;         //串口4信息
    static QString ConfigUart5;         //串口5信息
    static QString ConfigUart6;         //串口6信息
    static QString ServerAddr1;
    static QString ServerAddr2;
    static QString ServerAddr3;
    static QString ServerAddr4;
    static bool RespondOpen;
    static bool AutoUpdateOpen;
    static QString AutoUpdateIP;
    static QByteArray Key;
    static bool COM3ToServerOpen;
    static int SampleCmd;
    static int GetSamplestatusCmd;
    static int CleanSampleCmd;


    static void WriteConfig();          //读取配置文件,在main函数最开始加载程序载入
    static void ReadConfig();           //写入配置文件,在更改配置文件、程序关闭时调用
    static void WriteDefaultConfig();   //写入出厂设置配置文件
    static void ReadLocalNet();
    static void WriteLocalNet();

};

#endif // MYAPP_H
