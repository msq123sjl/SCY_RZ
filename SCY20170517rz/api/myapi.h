#ifndef MYAPI_H
#define MYAPI_H

#include <QObject>
#include "api/myapp.h"
#include "api/message.h"
#include <QList>
#include <QMutex>

class myAPI : public QObject
{
    Q_OBJECT
public:
    explicit myAPI(QObject *parent = 0);

    int   CountRecordIsExist(QString tableName,QString Time);
    int   CacheRecordIsExist(QString Code);
    bool TableIsExist(QString tableName);
    void RtdTableCreate(QString tableName);
    void CountDataTableCreate(QString tableName);
    double GetCountDataFromSql(QString tableName,QString StartTime,QString EndTime,QString field,QString funce);
    double GetCountDataFromSql1(QString tableName,QString StartTime,QString EndTime,QString field,QString funce);
    void CacheDataProc(double rtd,double total,QString flag,int dec,QString name,QString code,QString unit);
    void RtdProc();
     void AddEventInfo(QString TriggerType, QString TriggerContent);
     void AddEventInfoUser(QString TriggerContent); 
     void MessageFromCom(int porte);
     void ShowRtd();
     double AnalogConvert(double adValue,double RangeUp,double RangeLow,QString Signal);


     void MinsDataProc_WaterFlow(QString startTime,QString endTimee);
     void MinsDataProc_WaterPara(QString startTime,QString endTimee);
     void HourDataProc_WaterFlow(QString startTime,QString endTimee);
     void HourDataProc_WaterPara(QString startTime,QString endTimee);
     void DayDataProc(QString startTime,QString endTimee);

     void Insert_Message_Count(int CN,int flag,QString dte);
     void Insert_Message_Rtd(int flag,QString dte);
     void Update_Respond(QString QN,QString From);
     void Insert_Message_Received(QString QN,int CN,QString From,QString Content);
     void SendData_Master(int CN,int flage);
     void Protocol_1();
    void InsertList(QString str);
private:  
    Message a;   
    void Protocol_2(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_3(int port,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_4(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_5(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_6(int port,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_7(int port,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_8(int port,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_9(int port,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_10(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_11(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_12(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_13(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_14(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_15(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_16(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_17(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_18(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_19(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_20(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_21(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_22(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_23(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_24(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_25();
    void Protocol_26(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
    void Protocol_27(int port,int Address,int Dec,QString Name,QString Code,QString Unit);
//    void Protocol_28(int port,int Address,int Dec,QString Name,QString Code,QString Unit);

};

#endif // MYAPI_H
