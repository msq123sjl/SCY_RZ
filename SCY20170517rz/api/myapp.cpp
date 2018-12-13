#include "myapp.h"
#include "myhelper.h"
#include <QSettings>
#include <QTextStream>
QString myApp::SoftTitle="数据采集监测系统软件";
QString myApp::SoftVersion="V1.0";
QString myApp::CompanyName="江苏天泽环保科技有限公司";
QString myApp::ContactName="顾浩";
QString myApp::ContactTel="13800000000";
QString myApp::LastLoginer="";

QString myApp::CurrentUserName="system";
QString myApp::CurrentUserPwd="";
QString myApp::CurrentUserType="";
bool myApp::Login=false;
//程序运行过程中需要的全局变量
QString myApp::AppPath="";
int myApp::DeskWidth=800;
int myApp::DeskHeigth=480;
QString myApp::MN="88888880000001";
int myApp::StType=4;
int myApp::RtdInterval=600;
int myApp::MinInterval=10;
QString myApp::PW="123456";
QString myApp::AlarmTarget="1234567";
QString myApp::LocalIP="192.168.1.112";
QString myApp::Mask="255.255.255.0";
QString myApp::GateWay="192.168.1.1";
QString myApp::DHCP="0";
QString myApp::ReportTime="00|10";
int myApp::ReportTimeHour=0;
int myApp::ReportTimeMin=10;
int myApp::OverTime=30;
int myApp::ReCount=3;
int myApp::AlarmTime=30;
//波特率|数据位|校验位|停止位|协议|超时时间|通讯周期
QString myApp::ConfigUart1="9600|8|0|0|1000|1000";
QString myApp::ConfigUart2="9600|8|0|0|1000|1000";
QString myApp::ConfigUart3="9600|8|0|0|1000|1000";
QString myApp::ConfigUart4="9600|8|0|0|1000|1000";
QString myApp::ConfigUart5="9600|8|0|0|1000|1000";
QString myApp::ConfigUart6="9600|8|0|0|1000|1000";

QString myApp::ServerAddr1="false|192.168.1.200|8800";
QString myApp::ServerAddr2="false|192.168.1.200|8800";
QString myApp::ServerAddr3="false|192.168.1.200|8800";
QString myApp::ServerAddr4="false|192.168.1.200|8800";
bool myApp::RespondOpen=false;
bool myApp::AutoUpdateOpen=true;
QString myApp::AutoUpdateIP="192.168.1.200";
QByteArray myApp::Key;
bool myApp::COM3ToServerOpen=false;
int myApp::SampleCmd=0;
int myApp::GetSamplestatusCmd=0;
int myApp::CleanSampleCmd=0;
Com_para COM[6]={
    {"ttyS1",9600,8,0,0,1000,1000},
    {"ttyS2",9600,8,0,0,1000,1000},
    {"ttyS3",9600,8,0,0,1000,1000},
    {"ttyS4",9600,8,0,0,1000,1000},
    {"ttyS5",9600,8,0,0,1000,1000},
    {"ttyS6",9600,8,0,0,1000,1000},
};

Tcp_para TCP[4]={
    {false,"192.168.1.200","8800",false},
    {false,"192.168.1.200","8800",false},
    {false,"192.168.1.200","8800",false},
    {false,"192.168.1.200","8800",false},
};

Sampler sampler={0,100,1};

QextSerialPort *myCom[6];

void myApp::ReadConfig()
{
    QString fileName=myApp::AppPath+"config_scy.txt";
    //如果配置文件不存在,则以初始值继续运行
    //没有这个判断的话,配置赋值都为空
    if (!myHelper::FileIsExist(fileName)) {
        //对应中文转成正确的编码
        myApp::CurrentUserType=myApp::CurrentUserType.toLatin1();
        myApp::SoftTitle=myApp::SoftTitle.toLatin1();
        myApp::CompanyName=myApp::CompanyName.toLatin1();
        myApp::ContactName=myApp::ContactName.toLatin1();
        return ;
    }
    //创建配置文件操作对象
    QSettings *set=new QSettings(fileName,QSettings::IniFormat);

    set->beginGroup("AppConfig");

    myApp::SoftTitle=set->value("SoftTitle").toString();
    myApp::SoftVersion=set->value("SoftVersion").toString();
    myApp::CompanyName=set->value("CompanyName").toString();
    myApp::ContactName=set->value("ContactName").toString();
    myApp::ContactTel=set->value("ContactTel").toString();
    myApp::MN=set->value("MN").toString();
    myApp::StType=set->value("StType").toInt();
    myApp::RtdInterval=set->value("RtdInterval").toInt();
    myApp::MinInterval=set->value("MinInterval").toInt();
    myApp::PW=set->value("PW").toString();
    myApp::AlarmTarget=set->value("AlarmTarget").toString();
    myApp::LocalIP=set->value("LocalIP").toString();
    myApp::Mask=set->value("Mask").toString();
    myApp::GateWay=set->value("GateWay").toString();
    myApp::ReportTimeHour=set->value("ReportTimeHour").toInt();
    myApp::ReportTimeMin=set->value("ReportTimeMin").toInt();
    myApp::OverTime=set->value("OverTime").toInt();
    myApp::ReCount=set->value("ReCount").toInt();
    myApp::AlarmTime=set->value("AlarmTime").toInt();
    myApp::ConfigUart1=set->value("ConfigUart1").toString();
    myApp::ConfigUart2=set->value("ConfigUart2").toString();
    myApp::ConfigUart3=set->value("ConfigUart3").toString();
    myApp::ConfigUart4=set->value("ConfigUart4").toString();
    myApp::ConfigUart5=set->value("ConfigUart5").toString();
    myApp::ConfigUart6=set->value("ConfigUart6").toString();
    QStringList temp;
    for(int i=0;i<6;i++)
    {
        if(i==0)temp=myApp::ConfigUart1.split("|");
        else if(i==1)temp=myApp::ConfigUart2.split("|");
        else if(i==2)temp=myApp::ConfigUart3.split("|");
        else if(i==3)temp=myApp::ConfigUart4.split("|");
        else if(i==4)temp=myApp::ConfigUart5.split("|");
        else if(i==5)temp=myApp::ConfigUart6.split("|");
        COM[i].Baudrate=temp[0].toInt();
        COM[i].Databits=temp[1].toInt();
        COM[i].Parity=temp[2].toInt();
        COM[i].Stopbits=temp[3].toInt();
        COM[i].Timeout=temp[4].toInt();
        COM[i].Interval=temp[5].toInt();
    }
    myApp::ServerAddr1=set->value("ServerAddr1").toString();
    myApp::ServerAddr2=set->value("ServerAddr2").toString();
    myApp::ServerAddr3=set->value("ServerAddr3").toString();
    myApp::ServerAddr4=set->value("ServerAddr4").toString();
    for(int i=0;i<4;i++)
    {
        if(i==0)     temp=myApp::ServerAddr1.split("|");
        else if(i==1)temp=myApp::ServerAddr2.split("|");
        else if(i==2)temp=myApp::ServerAddr3.split("|");
        else if(i==3)temp=myApp::ServerAddr4.split("|");
        TCP[i].ServerOpen=(temp[0]=="true"?true:false);
        TCP[i].ServerIP=temp[1];
        TCP[i].ServerPort=temp[2];
    }

    myApp::RespondOpen=set->value("RespondOpen").toBool();
    myApp::AutoUpdateOpen=set->value("AutoUpdateOpen").toBool();
    myApp::AutoUpdateIP=set->value("AutoUpdateIP").toString();
    myApp::COM3ToServerOpen=set->value("COM3ToServerOpen").toBool();
    myApp::Key=set->value("Key").toByteArray();

    sampler.SampleMode=set->value("SampleMode").toInt();
    sampler.SampleFlow=set->value("SampleFlow").toInt();
    sampler.SampleStartBottle=set->value("SampleStartBottle").toInt();

    set->endGroup();
    delete set;
}

void myApp::WriteConfig()//保存配置信息
{    
    QString fileName=myApp::AppPath+"config_scy.txt";
    QSettings *set=new QSettings(fileName,QSettings::IniFormat);

    set->beginGroup("AppConfig");

    set->setValue("MN",myApp::MN);
    set->setValue("StType",myApp::StType);
    set->setValue("RtdInterval",myApp::RtdInterval);
    set->setValue("MinInterval",myApp::MinInterval);
    set->setValue("PW",myApp::PW);
    set->setValue("AlarmTarget",myApp::AlarmTarget);
    set->setValue("LocalIP",myApp::LocalIP);
    set->setValue("Mask",myApp::Mask);
    set->setValue("GateWay",myApp::GateWay);
    set->setValue("ReportTime",myApp::ReportTime);
    set->setValue("OverTime",myApp::OverTime);
    set->setValue("ReCount",myApp::ReCount);
    set->setValue("AlarmTime",myApp::AlarmTime);
    set->setValue("ConfigUart1",myApp::ConfigUart1);
    set->setValue("ConfigUart2",myApp::ConfigUart2);
    set->setValue("ConfigUart3",myApp::ConfigUart3);
    set->setValue("ConfigUart4",myApp::ConfigUart4);
    set->setValue("ConfigUart5",myApp::ConfigUart5);
    set->setValue("ConfigUart6",myApp::ConfigUart6);

    set->setValue("ServerAddr1",myApp::ServerAddr1);
    set->setValue("ServerAddr2",myApp::ServerAddr2);
    set->setValue("ServerAddr3",myApp::ServerAddr3);
    set->setValue("ServerAddr4",myApp::ServerAddr4);

    set->setValue("RespondOpen",myApp::RespondOpen);
    set->setValue("AutoUpdateOpen",myApp::AutoUpdateOpen);
    set->setValue("AutoUpdateIP",myApp::AutoUpdateIP);
    set->setValue("COM3ToServerOpen",myApp::COM3ToServerOpen);

    set->setValue("SampleMode",sampler.SampleMode);
    set->setValue("SampleFlow",sampler.SampleFlow);
    set->setValue("SampleStartBottle",sampler.SampleStartBottle);

    set->endGroup();
    delete set;

}

void myApp::WriteDefaultConfig()
{
    QString fileName=myApp::AppPath+"config_scy.txt";
    QSettings *set=new QSettings(fileName,QSettings::IniFormat);

    set->beginGroup("AppConfig");

    set->setValue("MN","88888880000001");
    set->setValue("StType",0);
    set->setValue("RtdInterval",600);
    set->setValue("MinInterval",10);
    set->setValue("PW","123456");
    set->setValue("AlarmTarget","1234567");
    set->setValue("LocalIP","192.168.1.112");
    set->setValue("Mask","255.255.255.0");
    set->setValue("GateWay","192.168.1.1");
    set->setValue("ReportTimeHour",0);
    set->setValue("ReportTimeMin",10);
    set->setValue("OverTime",30);
    set->setValue("ReCount",3);
    set->setValue("AlarmTime",30);
    set->setValue("ConfigUart1","9600|8|0|0|500|1000");
    set->setValue("ConfigUart2","9600|8|0|0|500|1000");
    set->setValue("ConfigUart3","9600|8|0|0|500|1000");
    set->setValue("ConfigUart4","9600|8|0|0|500|1000");
    set->setValue("ConfigUart5","9600|8|0|0|500|1000");
    set->setValue("ConfigUart6","9600|8|0|0|500|1000");

    set->setValue("ServerAddr1","false|192.168.1.200|8800");
    set->setValue("ServerAddr2","false|192.168.1.200|8800");
    set->setValue("ServerAddr3","false|192.168.1.200|8800");
    set->setValue("ServerAddr4","false|192.168.1.200|8800");

    set->setValue("RespondOpen","true");
    set->setValue("AutoUpdateOpen","true");
    set->setValue("AutoUpdateIP","192.168.1.200");
    set->setValue("COM3ToServerOpen","false");

    set->endGroup();
    delete set;

}

void myApp::ReadLocalNet()
{
    QString fileName=myApp::AppPath+"userinfo.txt";
    QSettings *set=new QSettings(fileName,QSettings::IniFormat);

    set->beginGroup("LOCAL_MACHINE");
    myApp::DHCP=set->value("DHCP").toString();
    myApp::GateWay=set->value("DefaultGateway").toString();
    myApp::LocalIP=set->value("IPAddress").toString();
    myApp::Mask=set->value("SubnetMask").toString();

    set->endGroup();
    delete set;

}

void myApp::WriteLocalNet()
{
    QString strAll;
    QFile readfile(myApp::AppPath+"userinfo.txt");
    if(readfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&readfile);
            strAll=stream.readAll();
    }
 readfile.close();
  QStringList strList;
 QFile writeFile(myApp::AppPath+"userinfo.txt");
 if(writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
 {
         QTextStream stream(&writeFile);
         strList=strAll.split("\n");
         for(int i=0;i<strList.count();i++)
         {
             if(i==strList.count()-1)
             {
                 //最后一行不需要换行
                 stream<<strList.at(i);
             }
             else
             {
                 stream<<strList.at(i)<<'\n';
             }

             if(strList.at(i).contains("[LOCAL_MACHINE]"))
             {

                 QString tempStr=strList.at(i+1);
                 tempStr.replace(0,tempStr.length(),"DHCP=\""+myApp::DHCP+'\"');
                 stream<<tempStr<<'\n';
                 tempStr=strList.at(i+2);
                 tempStr.replace(0,tempStr.length(),"DefaultGateway=\""+myApp::GateWay+'\"');
                 stream<<tempStr<<'\n';
                 tempStr=strList.at(i+3);
                 tempStr.replace(0,tempStr.length(),"IPAddress=\""+myApp::LocalIP+'\"');
                 stream<<tempStr<<'\n';
                 tempStr=strList.at(i+4);
                 tempStr.replace(0,tempStr.length(),"SubnetMask=\""+myApp::Mask+'\"');
                 stream<<tempStr<<'\n';
                 i+=4;
             }
         }
 }
 writeFile.close();
}




