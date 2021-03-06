#include "api/mythread.h"
#include "api/spi_drivers.h"
#include <QSqlQuery>
#include "api/gpio.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include "gpio.h"

void Uart1_Execute::run()
{   
    myAPI api;
    while(true)
    {
        api.MessageFromCom(0);
    }
}

void Uart2_Execute::run()
{
    myAPI api;
    while(true)
    {  
        if(myApp::COM3ToServerOpen){
            api.Protocol_1();
        }else{
            api.MessageFromCom(1);
        }

    }
}

void Uart3_Execute::run()
{
    myAPI api;
    while(true)
    {
        api.MessageFromCom(2);
    }
}

void Uart4_Execute::run()
{
    myAPI api;
    while(true)
    {
        api. MessageFromCom(3);
    }
}

void Uart5_Execute::run()
{
    myAPI api;
    while(true)
    {
        api.MessageFromCom(4);
    }
}

void Uart6_Execute::run()
{
    myAPI api;
    while(true)
    {
        api.MessageFromCom(5);
    }
}


void RtdProc::run()
{
    myAPI api;
    QString startTime,endTime;
    QDateTime Before,Before1,Before2,Now;
//    int fd;
//    int rc;

    Now=QDateTime::currentDateTime();
    Before=Now;
    Before1=Now;
    Before2=Now;
//    fd = open("/dev/watchdog", O_RDONLY);
    while(true)
    {
        Now=QDateTime::currentDateTime().addMSecs(25);
        if(Before.time().minute()!=Now.time().minute()){//处理实时数据
            Before=Now;
            api.RtdProc();
        }
        if(Before1.secsTo(Now)>=5){ //5S刷新一次
            Before1=Now;
            api.ShowRtd();
        }
        Now=QDateTime::currentDateTime().addMSecs(35);
        if(Before2.secsTo(Now)>=myApp::RtdInterval){//处理实时数据
            Before2=Now;
            endTime=Now.toString("yyyy-MM-dd hh:mm:ss");
            startTime=Now.addSecs(-myApp::RtdInterval).toString("yyyy-MM-dd hh:mm:ss");
            api.Insert_Message_Rtd(0,startTime);
        }

//        msleep(500);

//        rc = ioctl(fd, WDIOC_KEEPALIVE, 0);

        WDT_Feed();

    }
}

void Count::run()
{
    myAPI api;
    QString startTime,endTime;
    QDateTime Before_m,Before_h,Before_d,Now;

    do{
        Now=QDateTime::currentDateTime();
        Before_m=Now;
        Before_h=Now;
        Before_d=Now;
        sleep(1);
    }while(Now.time().second()!=0);//整分钟开始计时

    while(true)
    {



        Now=QDateTime::currentDateTime();
        if(Before_m.secsTo(Now)>=myApp::MinInterval*60){//处理分钟数据
            Before_m=Now;
            endTime=Now.toString("yyyy-MM-dd hh:mm:00");
            startTime=Now.addSecs(-myApp::MinInterval*60).toString("yyyy-MM-dd hh:mm:00");
            if(myApp::StType==2){
                api.MinsDataProc_WaterFlow(startTime,endTime);
                api.MinsDataProc_WaterPara(startTime,endTime);
            }
            api.Insert_Message_Count(2051,0,startTime);
        }

        if(Now.time().hour()!=Before_h.time().hour())//整点处理小时数据
        {
            Before_h=Now;
            startTime=Now.addSecs(-3600).toString("yyyy-MM-dd hh:00:00");
            endTime=Now.addSecs(-3600).toString("yyyy-MM-dd hh:59:59");
            if(myApp::StType==2){
                api.HourDataProc_WaterFlow(startTime,endTime);
                api.HourDataProc_WaterPara(startTime,endTime);
            }
            api.Insert_Message_Count(2061,myApp::RespondOpen,startTime);
        }

        if(Now.date().day()!=Before_d.date().day())//整点处理日数据
        {
            Before_d=Now;
            startTime=Now.addDays(-1).toString("yyyy-MM-dd 00:00:00");
            endTime=Now.addDays(-1).toString("yyyy-MM-dd 23:59:59");
            api.DayDataProc(startTime,endTime);
            api.Insert_Message_Count(2031,myApp::RespondOpen,startTime);
        }

        if(Power_Change)
        {
            Power_Change=0;
            startTime=Now.toString("yyyy-MM-dd hh:mm:ss");

            if(0==Power_New){
                api.Insert_Message_Count(3081,0,startTime);//无市电
            }
            else{
                api.Insert_Message_Count(3082,0,startTime);//有市电
            }

        }
//    msleep(500);
    }
}

#include "frmdiagnose.h"
void SendMessage::run()
{
    myAPI api;
    while(true)
    {
        sleep(myApp::OverTime);
        api.SendData_Master(2011,0);
        api.SendData_Master(2051,0);
        api.SendData_Master(2061,myApp::RespondOpen);
        api.SendData_Master(2031,myApp::RespondOpen);
        api.SendData_Master(3081,0);//无市电
        api.SendData_Master(3082,0);//有市电

    }
}


void DB_Clear::run()
{
        QString sql;
        QString temp;
        QDateTime Now,Before_d;
        Now=QDateTime::currentDateTime();
        Before_d=Now;

        while(true)
        {
            if(Now.date().day()!=Before_d.date().day())
            {
                QSqlQuery query;
                QSqlQuery query1;

                Before_d=Now;

                temp=Now.addDays(-10).toString("yyyyMMddhhmmsszzz");
                sql=QString("delete from [MessageSend] where [QN]<'%1'").arg(temp);
                query.exec(sql);
                sql=QString("delete from [MessageReceived] where [QN]<'%1'").arg(temp);
                query.exec(sql);

                query.exec("select [Code] from [ParaInfo]");
                while(query.next())
                {
                    temp=Now.addDays(-366).toString("yyyy-MM-dd hh:mm:ss");
                    sql=QString("delete from [Mins_%1] where [GetTime]<'%2'")
                            .arg(query.value(0).toString())
                            .arg(temp);
                    query1.exec(sql);

                    temp=Now.addDays(-30).toString("yyyyMMdd");
                    sql=QString("drop table Rtd_%1_%2")
                            .arg(query.value(0).toString())
                            .arg(temp);
                    query1.exec(sql);
                }

                query.exec("VACUUM");
            }
            Now=QDateTime::currentDateTime();
            sleep(600);
        }

}

extern float ad_value[8];
void SPI_Read_ad::run()
{
    myAPI api;
    QString Code;
    QString Name;
    QString  Unit;
    int Dec;
    int Port;
    double Rtd;
    QString flag="D";
    SPI_Init();

    while(true)
    {
        if(spi_read_ad()==true)
            flag="N";
        else
            flag="D";
        QSqlQuery query;

        query.exec("select * from [ParaInfo] where [UseChannel] like 'AN%'");
        while(query.next())
        {
            Name=query.value(0).toString();
            Code=query.value(1).toString();
            Unit=query.value(2).toString();
            Port=query.value(3).toString().right(1).toInt();
            Dec=query.value(15).toInt();
            Rtd=api.AnalogConvert(ad_value[Port],query.value(7).toDouble(),query.value(8).toDouble(),query.value(6).toString());
            api.CacheDataProc(Rtd,0,flag,Dec,Name,Code,Unit);
        }
        msleep(2000);
    }
}
