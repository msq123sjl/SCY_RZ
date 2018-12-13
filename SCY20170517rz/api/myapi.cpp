#include "myapi.h"
#include "api/myhelper.h"
#include "myapp.h"
#include <unistd.h>
#include <QStandardItemModel>
#include "message.h"
#include <QMutexLocker>
extern QextSerialPort *myCom[6];
extern Com_para COM[6];
extern Tcp_para TCP[4];
QList<QString> list;

myAPI::myAPI(QObject *parent) :
    QObject(parent)
{
}


//查找统计数据表内某时间段内某记录是否存在
int myAPI::CountRecordIsExist(QString tableName,QString Time)
{
    QSqlQuery query;
    QString sql;

    sql="select count(*) from "+tableName+" where [GetTime]='"+Time+"'";
    query.exec(sql);
    query.next();

    return (query.value(0).toInt());
}

//查找缓存数据表内某污染物记录是否存在
int myAPI::CacheRecordIsExist(QString Code)
{
    QSqlQuery query(QSqlDatabase::database("memory",true));
    QString sql;
    sql="select count(*) from [CacheRtd]";
    sql+=" where [Code]='"+Code+"'";
    query.exec(sql);
    query.next();

    return (query.value(0).toInt());
}

//查找某数据表是否存在
bool myAPI::TableIsExist(QString tableName)
{
    QSqlQuery query;
    QString sql;

    sql="select count(*) from sqlite_master where type='table' and name='"+tableName+"'";
    query.exec(sql);
    query.next();

    return (query.value(0).toBool());
}

//创建实时数据表
void myAPI::RtdTableCreate(QString tableName)
{
    QSqlQuery query;
    QString sql;
    sql="create table "+tableName+"(GetTime DATETIME(20),";
    sql+="Rtd NVARCHAR(20),Flag NVARCHAR(5),Total NVARCHAR(20))";
    query.exec(sql);

}
//创建统计数据表
void myAPI::CountDataTableCreate(QString tableName)
{
    QSqlQuery query;
    QString sql;
    sql="create table "+tableName+"(GetTime DATETIME(20),";
    sql+="Max NVARCHAR(20),Min NVARCHAR(20),Avg NVARCHAR(20),Cou NVARCHAR(20))";
    query.exec(sql);
}

//统计数据
double myAPI::GetCountDataFromSql(QString tableName,QString StartTime,QString EndTime,QString field,QString func)
{
    QSqlQuery query;
    QString sql;

    sql="select "+func+"(cast("+field+ " as double)) from "+'['+tableName+']';
    sql+=" where [GetTime]>='"+StartTime+"'"+" and [GetTime]<='"+EndTime+"'";
    sql+=" and [Flag]='N'";

    query.exec(sql);
    query.next();
    QByteArray str=query.value(0).toByteArray();
    double d_value=myHelper::Str_To_Double(str.data());
    return d_value;
}

//统计数据
double myAPI::GetCountDataFromSql1(QString tableName,QString StartTime,QString EndTime,QString field,QString func)
{
    QSqlQuery query;
    QString sql;

    sql="select "+func+"(cast("+field+ " as double)) from "+'['+tableName+']';
    sql+=" where [GetTime]>='"+StartTime+"'"+" and [GetTime]<='"+EndTime+"'";
    query.exec(sql);
    query.next();
    QByteArray str=query.value(0).toByteArray();
    double d_value=myHelper::Str_To_Double(str.data());
    return d_value;
}

void myAPI::CacheDataProc(double rtd,double total,QString flag,int dec,QString name,QString code,QString unit)
{
    QSqlQuery query(QSqlDatabase::database("memory",true));
    QString sql;
    char str[20];
    int errorNums=0;
    QString format=QString("%.%1f").arg(dec);
    QDateTime dt=QDateTime::currentDateTime();

    if(flag=="D"){
        query.exec(QString("select [ErrorNums] from [CacheRtd] where [Code]='%1'").arg(code));
        if(query.next()){
            errorNums=query.value(0).toInt();
            if(errorNums<5){
                errorNums++;
                sql = "update [CacheRtd] set [ErrorNums]='"+QString::number(errorNums)+"' where [Code]='"+code+"'";
                query.exec(sql);//更新显示数据
                return;
            }else{
                errorNums=5;
            }
        }
    }else{
        errorNums=0;
    }

    if(CacheRecordIsExist(code)==0){
            sql = "insert into [CacheRtd]";
            sql+="([GetTime],[Name],[Code],[Rtd],[Total],[Flag],[ErrorNums])values('";
            sql+= QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+ "','";
            sql+= name + "','";
            sql+= code + "','";
            sprintf(str,format.toLatin1().data(),rtd);
            sql+= QString(str) +" "+unit+ "','";
            if(total==0){
                sql+= "--','";
            }else{
                sprintf(str,format.toLatin1().data(),total);
                sql+= QString(str) + "','";
            }
            sql+= flag + "','";
            sql+= QString::number(errorNums) + "')";
            query.exec(sql);//插入显示数据

        }
    else{
            sql = "update [CacheRtd] set";
            sql+=" [GetTime]='"+dt.toString("yyyy-MM-dd hh:mm:ss")+"',";
            sql+= "[Name]='"+name + "',";
            sprintf(str,format.toLatin1().data(),rtd);
            sql+= "[Rtd]='"+QString(str) +" "+unit+ "',";
            if(total==0){
                sql+= "[Total]='--',";
            }else
            {
                sprintf(str,format.toLatin1().data(),total);
                sql+= "[Total]='"+QString(str) + "',";
            }
            sql+= "[Flag]='"+flag + "',";
            sql+= "[ErrorNums]='"+QString::number(errorNums)+"'";
            sql+= " where [Code]='"+code+"'";
            query.exec(sql);//更新显示数据

    }


}

void myAPI::RtdProc()
{
    QSqlQuery query(QSqlDatabase::database("memory",true));
    QString sql;
    QString temp,code,rtd,total,dt;
    sql="select * from [CacheRtd] where [flag]='N'";
    query.exec(sql);
    QSqlQuery query1;

    while(query.next())
    {
        dt=query.value(0).toString();
        code=query.value(2).toString();
        rtd=query.value(3).toString().split(" ")[0];
        total=query.value(4).toString();

        temp=QString("Rtd_%1_%2%3%4")
                .arg(code)
                .arg(dt.left(4))
                .arg(dt.mid(5,2))
                .arg(dt.mid(8,2));
        if(!TableIsExist(temp)){
            RtdTableCreate(temp);
        }
        sql = "insert into "+temp+"([GetTime],[Rtd],[Flag],[Total])values('";
        sql+= dt+ "','";
        sql+= rtd + "','";
        sql+= query.value(5).toString() + "','";
        sql+= total + "')";
        query1.exec(sql);//插入实时数据表

    }


}

//水流量数据统计--分钟数据
void myAPI::MinsDataProc_WaterFlow(QString startTime,QString endTime)
{
    bool max_flag,min_flag,avg_flag,cou_flag;
    QString code,format;
    double max_value=0,min_value=0,avg_value=0,cou_value=0;
    char str[20];
    QString sql;
    QString temp;
    double total1=0,total2=0;
    QSqlQuery query;
    QSqlQuery query1;

//    if(!TableIsExist("Mins_B01"))return;
    query.exec("select * from ParaInfo where [Code]='B01'");
    while(query.next())
    {
        code=query.value(1).toString();
        max_flag=query.value(11).toBool();
        min_flag=query.value(12).toBool();
        avg_flag=query.value(13).toBool();
        cou_flag=query.value(14).toBool();
        format=QString("%.%1f").arg(query.value(15).toInt());
        if(!TableIsExist(QString("Mins_%1").arg(code))){
            CountDataTableCreate(QString("Mins_%1").arg(code));
        }
        if(CountRecordIsExist(QString("Mins_%1").arg(code),startTime)==true)continue;
        temp=QString("Rtd_%1_%2%3%4")
                .arg(code)
                .arg(startTime.left(4))
                .arg(startTime.mid(5,2))
                .arg(startTime.mid(8,2));
        if(!TableIsExist(temp))continue;

        max_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MAX");
        min_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MIN");
        avg_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","AVG");

        sql="select [Total] from "+temp+" where [GetTime]>='"+startTime+"' and [GetTime]<='"+endTime+"'  order by [GetTime] asc";
        query1.exec(sql);
        query1.first();
        total1=myHelper::Str_To_Double(query1.value(0).toByteArray().data());
        query1.last();
        total2=myHelper::Str_To_Double(query1.value(0).toByteArray().data());
        cou_value=total2-total1;

        sql = "insert into Mins_"+code;
        sql+= "([GetTime],[Max],[Min],[Avg],[Cou])values('";
        sql+= startTime+ "','";
        if(max_flag){
            sprintf(str,format.toLatin1().data(),max_value);
            sql+= QString(str) + "','";
        }else{
            sql+= "--','";
        }
        if(min_flag){
            sprintf(str,format.toLatin1().data(),min_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(avg_flag){
            sprintf(str,format.toLatin1().data(),avg_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(cou_flag){
            sprintf(str,format.toLatin1().data(),cou_value);
            sql+= QString(str) + "')";
        }else{
            sql+="--')";
        }

        query1.exec(sql);//插入分钟数据表

    }
}

//水污染因子数据统计--分钟数据
void myAPI::MinsDataProc_WaterPara(QString startTime,QString endTime)
{
    bool max_flag,min_flag,avg_flag,cou_flag;
    QString code,format;
    double max_value=0,min_value=0,avg_value=0,cou_value;
    char str[20];
    QString sql;
    QString temp;
    double total=0;
    QSqlQuery query;
    QSqlQuery query1;
    query.exec("select * from ParaInfo where [Code]<>'B01'");
//    query.exec("select * from ParaInfo where [Code]='B01'");

    while(query.next())
    {
        code=query.value(1).toString();
        if(!TableIsExist(QString("Mins_%1").arg(code))){
            CountDataTableCreate(QString("Mins_%1").arg(code));

        }
        if(CountRecordIsExist(QString("Mins_%1").arg(code),startTime)==true)continue;
        max_flag=query.value(11).toBool();
        min_flag=query.value(12).toBool();
        avg_flag=query.value(13).toBool();
        cou_flag=query.value(14).toBool();
        format=QString("%.%1f").arg(query.value(15).toInt());
        temp=QString("Rtd_%1_%2%3%4")
                .arg(code)
                .arg(startTime.left(4))
                .arg(startTime.mid(5,2))
                .arg(startTime.mid(8,2));
        if(!TableIsExist(temp))continue;
        max_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MAX");
        min_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MIN");
        avg_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","AVG");
        sql="select [Cou] from [Mins_B01] where [GetTime]='"+startTime+"'";
        query1.exec(sql);
        query1.next();
        total=myHelper::Str_To_Double(query1.value(0).toByteArray().data());
        cou_value=total*avg_value*0.001;
        qDebug()<<QString("Mins_%1_total=%2").arg(code).arg(total);
        sql = "insert into Mins_"+code;
        sql+= "([GetTime],[Max],[Min],[Avg],[Cou])values('";
        sql+= startTime+ "','";
        if(max_flag){
            sprintf(str,format.toLatin1().data(),max_value);
            sql+= QString(str) + "','";
        }else{
            sql+= "--','";
        }
        if(min_flag){
            sprintf(str,format.toLatin1().data(),min_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(avg_flag){
            sprintf(str,format.toLatin1().data(),avg_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(cou_flag){
            sprintf(str,format.toLatin1().data(),cou_value);
            sql+= QString(str) + "')";
        }else{
            sql+="--')";
        }

        query1.exec(sql);//插入分钟数据表

    }

}

//水流量数据统计--小时数据
void myAPI::HourDataProc_WaterFlow(QString startTime,QString endTime)
{
    bool max_flag,min_flag,avg_flag,cou_flag;
    QString code,format;
    double max_value=0,min_value=0,avg_value=0,cou_value;
    char str[20];
    QString sql;
    QString temp;
    double total1=0,total2=0;
    QSqlQuery query;
    QSqlQuery query1;
    query.exec("select * from ParaInfo where [Code]='B01'");
    while(query.next())
    {
        code=query.value(1).toString();
        if(!TableIsExist(QString("Hour_%1").arg(code))){
            CountDataTableCreate(QString("Hour_%1").arg(code));
        }
        if(CountRecordIsExist(QString("Hour_%1").arg(code),startTime)==true)continue;
        max_flag=query.value(11).toBool();
        min_flag=query.value(12).toBool();
        avg_flag=query.value(13).toBool();
        cou_flag=query.value(14).toBool();
        format=QString("%.%1f").arg(query.value(15).toInt());
        temp=QString("Rtd_%1_%2%3%4")
                .arg(code)
                .arg(startTime.left(4))
                .arg(startTime.mid(5,2))
                .arg(startTime.mid(8,2));
        if(!TableIsExist(temp))continue;
        max_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MAX");
        min_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MIN");
        avg_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","AVG");
        sql="select [Total] from "+temp+" where [GetTime]>='"+startTime+"' and [GetTime]<='"+endTime+"'  order by [GetTime] asc";

        query1.exec(sql);
        query1.first();
        total1=myHelper::Str_To_Double(query1.value(0).toByteArray().data());
        query1.last();
        total2=myHelper::Str_To_Double(query1.value(0).toByteArray().data());
        cou_value=total2-total1;

        sql = "insert into Hour_"+code;
        sql+= "([GetTime],[Max],[Min],[Avg],[Cou])values('";
        sql+= startTime+ "','";
        if(max_flag){
            sprintf(str,format.toLatin1().data(),max_value);
            sql+= QString(str) + "','";
        }else{
            sql+= "--','";
        }
        if(min_flag){
            sprintf(str,format.toLatin1().data(),min_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(avg_flag){
            sprintf(str,format.toLatin1().data(),avg_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(cou_flag){
            sprintf(str,format.toLatin1().data(),cou_value);
            sql+= QString(str) + "')";
        }else{
            sql+="--')";
        }
        query1.exec(sql);//插入小时数据表
    }
}

//水污染因子数据统计--小时数据
void myAPI::HourDataProc_WaterPara(QString startTime,QString endTime)
{
    bool max_flag,min_flag,avg_flag,cou_flag;
    QString code,format;
    double max_value=0,min_value=0,avg_value=0,cou_value=0;
    char str[20];
    QString sql;
    QString temp;
    double total=0;
    QSqlQuery query;
    QSqlQuery query1;
    query.exec("select * from ParaInfo where [Code]<>'B01'");
    while(query.next())
    {
        code=query.value(1).toString();
        if(!TableIsExist(QString("Hour_%1").arg(code))){
            CountDataTableCreate(QString("Hour_%1").arg(code));
        }
        if(CountRecordIsExist(QString("Hour_%1").arg(code),startTime)==true)continue;
        max_flag=query.value(11).toBool();
        min_flag=query.value(12).toBool();
        avg_flag=query.value(13).toBool();
        cou_flag=query.value(14).toBool();
        format=QString("%.%1f").arg(query.value(15).toInt());
        temp=QString("Rtd_%1_%2%3%4")
                .arg(code)
                .arg(startTime.left(4))
                .arg(startTime.mid(5,2))
                .arg(startTime.mid(8,2));
        if(!TableIsExist(temp))continue;
        max_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MAX");
        min_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","MIN");
        avg_value=GetCountDataFromSql(temp,startTime,endTime,"Rtd","AVG");
        sql="select [Cou] from [Hour_B01] where [GetTime]='"+startTime+"'";
        qDebug()<<QString(sql);
        query1.exec(sql);
        query1.next();
        total=myHelper::Str_To_Double(query1.value(0).toByteArray().data());
        cou_value=total*avg_value*0.001;
        qDebug()<<QString("%1_Hour_total=%2").arg(code).arg(total);
        sql = "insert into Hour_"+code;
        sql+= "([GetTime],[Max],[Min],[Avg],[Cou])values('";
        sql+= startTime+ "','";
        if(max_flag){
            sprintf(str,format.toLatin1().data(),max_value);
            sql+= QString(str) + "','";
        }else{
            sql+= "--','";
        }
        if(min_flag){
            sprintf(str,format.toLatin1().data(),min_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(avg_flag){
            sprintf(str,format.toLatin1().data(),avg_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(cou_flag){
            sprintf(str,format.toLatin1().data(),cou_value);
            sql+= QString(str) + "')";
        }else{
            sql+="--')";
        }
        query1.exec(sql);//插入小时数据表
    }
}

void myAPI::DayDataProc(QString startTime,QString endTime)
{
    bool max_flag,min_flag,avg_flag,cou_flag;
    QString code,format;
    double max_value=0,min_value=0,avg_value=0,cou_value;
    char str[20];
    QString sql;
    QString temp;
    QSqlQuery query;
    QSqlQuery query1;
    query.exec("select * from ParaInfo");
    while(query.next())
    {
        code=query.value(1).toString();
        if(!TableIsExist(QString("Day_%1").arg(code))){
            CountDataTableCreate(QString("Day_%1").arg(code));
        }
        if(CountRecordIsExist(QString("Day_%1").arg(code),startTime)==true)continue;
        max_flag=query.value(11).toBool();
        min_flag=query.value(12).toBool();
        avg_flag=query.value(13).toBool();
        cou_flag=query.value(14).toBool();
        format=QString("%.%1f").arg(query.value(15).toInt());
        temp=QString("Hour_%1").arg(code);
        if(!TableIsExist(temp))continue;
        max_value=GetCountDataFromSql1(temp,startTime,endTime,"Max","MAX");
        min_value=GetCountDataFromSql1(temp,startTime,endTime,"Min","MIN");
        avg_value=GetCountDataFromSql1(temp,startTime,endTime,"Avg","AVG");
        cou_value=GetCountDataFromSql1(temp,startTime,endTime,"Cou","SUM");  //注意单位

        sql = "insert into Day_"+code;
        sql+= "([GetTime],[Max],[Min],[Avg],[Cou])values('";
        sql+= startTime+ "','";
        if(max_flag){
            sprintf(str,format.toLatin1().data(),max_value);
            sql+= QString(str) + "','";
        }else{
            sql+= "--','";
        }
        if(min_flag){
            sprintf(str,format.toLatin1().data(),min_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(avg_flag){
            sprintf(str,format.toLatin1().data(),avg_value);
            sql+= QString(str) + "','";
        }else{
            sql+="--','";
        }
        if(cou_flag){
            sprintf(str,format.toLatin1().data(),cou_value);
            sql+= QString(str) + "')";
        }else{
            sql+="--')";
        }
        query1.exec(sql);//插入日数据表
    }
}


//添加事件记录
void myAPI::AddEventInfo(QString TriggerType, QString TriggerContent)
{
    QString sql = "insert into [EventInfo]([TriggerTime],[TriggerType],";
    sql+="[TriggerContent],[TriggerUser])values('";
    sql += QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "','";
    sql += TriggerType + "','";
    sql += TriggerContent + "','";
    sql += myApp::CurrentUserName + "')";

    QSqlQuery query;
    query.exec(sql);
}

void myAPI::AddEventInfoUser(QString TriggerContent)
{
  this->AddEventInfo("用户操作", TriggerContent);
}


//读取监测污染物信息
QStandardItemModel  *model_rtd;
void myAPI::ShowRtd()
{
    QSqlQuery query(QSqlDatabase::database("memory",true));
    query.exec("select [Name],[Rtd],[Total],[Flag] from [CacheRtd]");
    int rows=0;
    while(query.next())
    {
        for(int i=0;i<model_rtd->columnCount();i++){
            model_rtd->setItem(rows,i,new QStandardItem(query.value(i).toString()));
//            model_rtd->item(rows,i)->setForeground(QBrush(QColor(0, 0, 0)));    //设置字符颜色
            model_rtd->item(rows,i)->setTextAlignment(Qt::AlignCenter);           //设置字符位置
            if(rows%2==1){
                if(query.value(i).toString()=="T"){
                    model_rtd->item(rows,i)->setBackground(QBrush(QColor(150,0,0,80)));
                }else{
                    model_rtd->item(rows,i)->setBackground(QBrush(QColor(0,255,0,80)));
                }
            }
        }
        rows++;
    }

}

void myAPI::InsertList(QString str)
{
    if(list.size()>=20){
        list.clear();
    }
    list.append(str);

}

void myAPI::Insert_Message_Count(int CN,int flag,QString dt)
{
    QString content;
    bool max_flag,min_flag,avg_flag,cou_flag;
    QByteArray ch;
    QString code;
    QString sql;
    QString QN=QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    content="ST="+ST_to_Str[myApp::StType]+";QN="+QN;
    content+=";CN="+QString::number(CN)+";PW=123456;MN="+myApp::MN;
    content+=";Flag="+QString::number(flag)+";CP=&&DataTime="+QString("%1%2%3%4%5%6")
            .arg(dt.mid(0,4))
            .arg(dt.mid(5,2))
            .arg(dt.mid(8,2))
            .arg(dt.mid(11,2))
            .arg(dt.mid(14,2))
            .arg(dt.mid(17,2));
    QSqlQuery query;
    QSqlQuery query1;

    if(CN==3081||CN==3082){
        content+="&&";
        ch.resize(4);
        int len=content.size();
        int crc=myHelper::CRC16_GB212(content.toLatin1().data(),len);
        sprintf(ch.data(),"%.4X",crc);
        content.append(ch+"\r\n");
        ch[0] = (len/1000)+'0';
        ch[1] = (len%1000/100)+'0';
        ch[2] = (len%100/10)+'0';
        ch[3] = (len%10)+'0';
        content.insert(0,"##");
        content.insert(2,ch);
        sql = "insert into MessageSend([QN],[CN],[Content],";
        sql+="[Target1_SendTimes],[Target2_SendTimes],[Target3_SendTimes],[Target4_SendTimes],[Target5_SendTimes],";
        sql+="[Target1_IsRespond],[Target2_IsRespond],[Target3_IsRespond],[Target4_IsRespond],[Target5_IsRespond])values('";
        sql+= QN+ "','"+QString::number(CN)+"','"+content+"','0','0','0','0','0','--','--','--','--','--')";
        query.exec(sql);//插入发送数据表
        return;
    }
        query.exec("select * from ParaInfo");
        while(query.next())
        {
            code=query.value(1).toString();
            max_flag=query.value(11).toBool();
            min_flag=query.value(12).toBool();
            avg_flag=query.value(13).toBool();
            cou_flag=query.value(14).toBool();
            if(CN==2051){
                query1.exec(QString("select * from [Mins_%1] where [GetTime]='%2'").arg(code).arg(dt));

            }else if(CN==2061){
                query1.exec(QString("select * from [Hour_%1] where [GetTime]='%2'").arg(code).arg(dt));

            }else if(CN==2031){
                query1.exec(QString("select * from [Day_%1] where [GetTime]='%2'").arg(code).arg(dt));

            }else {
                return;
            }

            while(query1.next())
            {
                if(max_flag)
                {
                    content+=';'+code+"-Max="+query1.value(1).toString();
                }
                if(min_flag)
                {
                    content+=','+code+"-Min="+query1.value(2).toString();
                }
                if(avg_flag)
                {
                    content+=','+code+"-Avg="+query1.value(3).toString();
                }
                if(cou_flag)
                {
                    content+=','+code+"-Cou="+query1.value(4).toString();
                }

            }

        }

    content+="&&";
//    if(CN==3081||CN==3082)qDebug()<<content;

    ch.resize(4);
    int len=content.size();
    int crc=myHelper::CRC16_GB212(content.toLatin1().data(),len);
    sprintf(ch.data(),"%.4X",crc);
    content.append(ch+"\r\n");
    ch[0] = (len/1000)+'0';
    ch[1] = (len%1000/100)+'0';
    ch[2] = (len%100/10)+'0';
    ch[3] = (len%10)+'0';
    content.insert(0,"##");
    content.insert(2,ch);
    if(flag&0x01){
        sql = "insert into MessageSend([QN],[CN],[Content],";
        sql+="[Target1_SendTimes],[Target2_SendTimes],[Target3_SendTimes],[Target4_SendTimes],[Target5_SendTimes],";
        sql+="[Target1_IsRespond],[Target2_IsRespond],[Target3_IsRespond],[Target4_IsRespond],[Target5_IsRespond])values('";
        sql+= QN+ "','"+QString::number(CN)+"','"+content+"','0','0','0','0','0','false','false','false','false','false')";
    }
    else{
        sql = "insert into MessageSend([QN],[CN],[Content],";
        sql+="[Target1_SendTimes],[Target2_SendTimes],[Target3_SendTimes],[Target4_SendTimes],[Target5_SendTimes],";
        sql+="[Target1_IsRespond],[Target2_IsRespond],[Target3_IsRespond],[Target4_IsRespond],[Target5_IsRespond])values('";
        sql+= QN+ "','"+QString::number(CN)+"','"+content+"','0','0','0','0','0','--','--','--','--','--')";
    }
    query.exec(sql);//插入发送数据表

}

void myAPI::Insert_Message_Rtd(int flag,QString dt)
{
    QString content;
    QSqlQuery query(QSqlDatabase::database("memory",true));
    QByteArray ch;
    QString sql;
    QString QN=QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    QSqlQuery query1;
    content="QN="+QN+";ST="+ST_to_Str[myApp::StType];
    content+=";CN=2011;PW=123456;MN="+myApp::MN+";Flag="+QString::number(flag)+";CP=&&DataTime="+QString("%1%2%3%4%5%6")
            .arg(dt.mid(0,4))
            .arg(dt.mid(5,2))
            .arg(dt.mid(8,2))
            .arg(dt.mid(11,2))
            .arg(dt.mid(14,2))
            .arg(dt.mid(17,2));
    query.exec("select * from [CacheRtd]");
    while(query.next())
    {
        content+=";"+query.value(2).toString()+"-Rtd="+query.value(3).toString().split(" ")[0];
        content+=","+query.value(2).toString()+"-Flag="+query.value(5).toString();
    }
    content+="&&";

    ch.resize(4);
    int len=content.size();
    int crc=myHelper::CRC16_GB212(content.toLatin1().data(),len);
    sprintf(ch.data(),"%.4X",crc);
    content.append(ch+"\r\n");
    ch[0] = (len/1000)+'0';
    ch[1] = (len%1000/100)+'0';
    ch[2] = (len%100/10)+'0';
    ch[3] = (len%10)+'0';
    content.insert(0,"##");
    content.insert(2,ch);

    if(flag&0x01){
        sql = "insert into MessageSend([QN],[CN],[Content],";
        sql+="[Target1_SendTimes],[Target2_SendTimes],[Target3_SendTimes],[Target4_SendTimes],[Target5_SendTimes],";
        sql+="[Target1_IsRespond],[Target2_IsRespond],[Target3_IsRespond],[Target4_IsRespond],[Target5_IsRespond])values('";
        sql+= QN+ "','2011','"+content+"','0','0','0','0','0','false','false','false','false','false')";
    }
    else{
        sql = "insert into MessageSend([QN],[CN],[Content],";
        sql+="[Target1_SendTimes],[Target2_SendTimes],[Target3_SendTimes],[Target4_SendTimes],[Target5_SendTimes],";
        sql+="[Target1_IsRespond],[Target2_IsRespond],[Target3_IsRespond],[Target4_IsRespond],[Target5_IsRespond])values('";
        sql+= QN+ "','2011','"+content+"','0','0','0','0','0','--','--','--','--','--')";
    }
    query1.exec(sql);//插入发送实时数据表

}

void myAPI::Update_Respond(QString QN,QString From)
{
    QSqlQuery query;
    QString sql;

    if(From==QString("%1:%2").arg(TCP[0].ServerIP).arg(TCP[0].ServerPort)){
        sql = "update [MessageSend] set [Target1_IsRespond]='true' where [QN]='"+QN+"'";
        query.exec(sql);//更新接收标记
    }else if(From==QString("%1:%2").arg(TCP[1].ServerIP).arg(TCP[1].ServerPort)){
        sql = "update [MessageSend] set [Target2_IsRespond]='true' where [QN]='"+QN+"'";
        query.exec(sql);//更新接收标记
    }else if(From==QString("%1:%2").arg(TCP[2].ServerIP).arg(TCP[2].ServerPort)){
        sql = "update [MessageSend] set [Target3_IsRespond]='true' where [QN]='"+QN+"'";
        query.exec(sql);//更新接收标记
    }else if(From==QString("%1:%2").arg(TCP[3].ServerIP).arg(TCP[3].ServerPort)){
        sql = "update [MessageSend] set [Target4_IsRespond]='true' where [QN]='"+QN+"'";
        query.exec(sql);//更新接收标记
    }else if(From=="COM3"){
        sql = "update [MessageSend] set [Target5_IsRespond]='true' where [QN]='"+QN+"'";
        query.exec(sql);//更新接收标记
    }

    sql = "update [MessageReceived] set [IsProcessed]='true' where [QN]='"+QN+"'";
    query.exec(sql);//更新处理标记

}

void myAPI::Insert_Message_Received(QString QN,int CN,QString From,QString Content) //多线程操作
{
    QString sql;
    sql = QString("insert into MessageReceived([ReceivedTime],[QN],[CN],[From],[IsProcessed],[Content])values('%1','%2','%3','%4','false','%5')")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(QN)
            .arg(CN)
            .arg(From)
            .arg(Content);
    QSqlQuery query;
    query.exec(sql);//插入接收数据表

}


extern QTcpSocket *tcpSocket1;
extern QTcpSocket *tcpSocket2;
extern QTcpSocket *tcpSocket3;
extern QTcpSocket *tcpSocket4;
extern Tcp_para     TCP[4];
extern QextSerialPort *myCom[6];

void myAPI::SendData_Master(int CN,int flag)   //单线程操作
{
    QString sql,data;
    int sendnumber=0;
    QSqlQuery query;
    QSqlQuery query1;
    if(TCP[0].ServerOpen==true)
    {
        sendnumber=0;
        if(flag&0x01)
        {
            sql=QString("select [QN],[Content],[Target1_SendTimes] from [MessageSend] where [CN]='%1' and [Target1_SendTimes]<'%2' and [Target1_IsRespond]='false'")
                    .arg(CN).arg(myApp::ReCount);
        }else{
            sql=QString("select [QN],[Content], [Target1_SendTimes] from [MessageSend] where [CN]='%1' and [Target1_SendTimes]<'1'")
                    .arg(CN);
        }
        query.exec(sql);
        while(query.next())
        {
            sendnumber++;
            if(sendnumber>5){
                sendnumber=0;
                return;
            }
            if(TCP[0].isConnected){
                 tcpSocket1->write(query.value(1).toByteArray());
                 tcpSocket1->flush();
                 sql = QString("update [MessageSend] set [Target1_SendTimes]='%1' where [QN]='%2'")
                         .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                 query1.exec(sql);//更新发送次数
                 data=QString("Tx[%1:%2]:%3").arg(TCP[0].ServerIP).arg(TCP[0].ServerPort).arg(query.value(1).toString());
                 InsertList(data);

            }else{
                if(CN==2011||CN==2051){//实时数据和分钟数据不自动补发
                    sql = QString("update [MessageSend] set [Target1_SendTimes]='%1' where [QN]='%2'")
                            .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                    query1.exec(sql);//更新发送次数
                }
            }

        }
    }

    if(TCP[1].ServerOpen==true)
    {
        sendnumber=0;
        if(flag&0x01)
        {
            sql=QString("select [QN],[Content],[Target2_SendTimes] from [MessageSend] where [CN]='%1' and [Target2_SendTimes]<'%2' and [Target2_IsRespond]='false'")
                    .arg(CN).arg(myApp::ReCount);
        }else{
            sql=QString("select [QN],[Content],[Target2_SendTimes] from [MessageSend] where [CN]='%1' and [Target2_SendTimes]<'1'")
                    .arg(CN);
        }
        query.exec(sql);
        while(query.next())
        {
            sendnumber++;
            if(sendnumber>5){
                sendnumber=0;
                return;
            }

/////////////////针对服务端防火墙长时间空闲自动断开专用///////////////////////////
//            int result_va=0;
//            result_va= tcpSocket2->write("##12342345465765784\r\n");//加一个心跳包

//            qDebug()<<QString("result_out=%1").arg(result_va);
//            sleep(25);

//            if(TCP[1].isConnected==false)sleep(10);

            if(TCP[1].isConnected){
                 tcpSocket2->write(query.value(1).toByteArray());
                 tcpSocket2->flush();
                 sql = QString("update [MessageSend] set [Target2_SendTimes]='%1' where [QN]='%2'")
                         .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                 query1.exec(sql);//更新发送次数
                 data=QString("Tx[%1:%2]:%3").arg(TCP[1].ServerIP).arg(TCP[1].ServerPort).arg(query.value(1).toString());
                 InsertList(data);

            }else{
                if(CN==2011||CN==2021||CN==2051){
                    sql = QString("update [MessageSend] set [Target2_SendTimes]='%1' where [QN]='%2'")
                            .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                    query1.exec(sql);//更新发送次数
                }
            }
        }
    }

    if(TCP[2].ServerOpen==true)
    {
        sendnumber=0;
        if(flag&0x01)
        {
            sql=QString("select [QN],[Content],[Target3_SendTimes] from [MessageSend] where [CN]='%1' and [Target3_SendTimes]<'%2' and [Target3_IsRespond]='false'")
                    .arg(CN).arg(myApp::ReCount);
        }else{
            sql=QString("select [QN],[Content],[Target3_SendTimes] from [MessageSend] where [CN]='%1' and [Target3_SendTimes]<'1'")
                    .arg(CN);
        }
        query.exec(sql);
        while(query.next())
        {
            sendnumber++;
            if(sendnumber>5){
                sendnumber=0;
                return;
            }

            if(TCP[2].isConnected){
                 tcpSocket3->write(query.value(1).toByteArray());
                 tcpSocket3->flush();
                 sql = QString("update [MessageSend] set [Target3_SendTimes]='%1' where [QN]='%2'")
                         .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                 query1.exec(sql);//更新发送次数
                 data=QString("Tx[%1:%2]:%3").arg(TCP[2].ServerIP).arg(TCP[2].ServerPort).arg(query.value(1).toString());
                 InsertList(data);

            }else{
                if(CN==2011||CN==2021||CN==2051){
                    sql = QString("update [MessageSend] set [Target3_SendTimes]='%1' where [QN]='%2'")
                            .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                    query1.exec(sql);//更新发送次数
                }
            }
        }
    }

    if(TCP[3].ServerOpen==true)
    {
        sendnumber=0;
        if(flag&0x01)
        {
            sql=QString("select [QN],[Content],[Target4_SendTimes] from [MessageSend] where [CN]='%1' and [Target4_SendTimes]<'%2' and [Target4_IsRespond]='false'")
                    .arg(CN).arg(myApp::ReCount);
        }else{
            sql=QString("select [QN],[Content],[Target4_SendTimes] from [MessageSend] where [CN]='%1' and [Target4_SendTimes]<'1'")
                    .arg(CN);
        }
        query.exec(sql);
        while(query.next())
        {
            sendnumber++;
            if(sendnumber>5){
                sendnumber=0;
                return;
            }

            if(TCP[3].isConnected){
                 tcpSocket4->write(query.value(1).toByteArray());
                 tcpSocket4->flush();
                 sql = QString("update [MessageSend] set [Target4_SendTimes]='%1' where [QN]='%2'")
                         .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                 query1.exec(sql);//更新发送次数
                 data=QString("Tx[%1:%2]:%3").arg(TCP[3].ServerIP).arg(TCP[3].ServerPort).arg(query.value(1).toString());
                 InsertList(data);

            }else{
                if(CN==2011||CN==2021||CN==2051){
                    sql = QString("update [MessageSend] set [Target4_SendTimes]='%1' where [QN]='%2'")
                            .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                    query1.exec(sql);//更新发送次数
                }
            }
        }
    }

    if(myApp::COM3ToServerOpen==true)
    {
        sendnumber=0;
        if(flag&0x01)
        {
            sql=QString("select [QN],[Content],[Target5_SendTimes] from [MessageSend] where [CN]='%1' and [Target5_SendTimes]<'%2' and [Target5_IsRespond]='false'")
                    .arg(CN).arg(myApp::ReCount);
        }else{
            sql=QString("select [QN],[Content],[Target5_SendTimes] from [MessageSend] where [CN]='%1' and [Target5_SendTimes]<'1'")
                    .arg(CN);
        }
        query.exec(sql);
        while(query.next())
        {
            sendnumber++;
            if(sendnumber>5){
                sendnumber=0;
                return;
            }

           if(myCom[1]->isOpen()){
               myCom[1]->write(query.value(1).toByteArray());
               myCom[1]->flush();
               sql = QString("update [MessageSend] set [Target5_SendTimes]='%1' where [QN]='%2'")
                       .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
               query1.exec(sql);//更新发送次数
               data=QString("Tx[COM3]:%1").arg(query.value(1).toString());
               InsertList(data);

           }else{
                if(CN==2011||CN==2021||CN==2051){
                    sql = QString("update [MessageSend] set [Target5_SendTimes]='%1' where [QN]='%2'")
                            .arg(query.value(2).toInt()+1).arg(query.value(0).toString());
                    query1.exec(sql);//更新发送次数
                }
           }
        }
    }
}
//**************************************************************************************
//模拟采集电压转成实际值
double myAPI::AnalogConvert(double adValue,double RangeUp,double RangeLow,QString Signal)
{
    double realValue=0;
    //采样电阻200欧姆（认证使用）
    if(Signal=="4-20mA")
    {
        adValue=1.0050251*adValue+0.006;
        realValue=(RangeUp-RangeLow)*(adValue*10-8)/32+RangeLow;
    }
    else if(Signal=="0-20mA"){
        realValue=(RangeUp-RangeLow)*(adValue*10-0)/40+RangeLow;
    }

////采样电阻100欧姆
//    if(Signal=="4-20mA")
//    {
//        realValue=(RangeUp-RangeLow)*(adValue*10-4)/16+RangeLow;
//    }
//    else if(Signal=="0-20mA"){
//        realValue=(RangeUp-RangeLow)*(adValue*10-0)/20+RangeLow;
//    }
    else if(Signal=="1-5V"){
        realValue=(RangeUp-RangeLow)*(adValue-1)/4+RangeLow;
    }
    else if(Signal=="0-5V"){
        realValue=(RangeUp-RangeLow)*(adValue-0)/5+RangeLow;
    }


    if(realValue<RangeLow){
        realValue=RangeLow;
    }
//    if(realValue>RangeUp){
//        realValue=RangeUp;
//    }

    return realValue;
}

//**************************************************************************************
//GB212协议
void myAPI::Protocol_1()
{
    int  tt=0;
    QByteArray temp;
    QByteArray readbuf;
    do{
        temp=myCom[1]->readAll();
        readbuf+=temp;
        if(temp.size()==0)tt++;
        else tt=0;
        if(tt>COM[1].Timeout/20)break;
        usleep(20000);
    }while(!readbuf.endsWith("\r\n"));

    if(tt<=COM[1].Timeout/20&&readbuf.size()>0){
        qDebug()<<QString("COM3 received:%1").arg(readbuf.data());
        a.messageProc(readbuf,myCom[1],NULL);
    }
}


//天泽VOCs
void myAPI::Protocol_2(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    volatile uchar s[4];
    float f1,f2;
    QByteArray readbuf;
    QByteArray sendbuf;

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x10;
    int check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    if(readbuf.length()>=37){
        qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
        s[0]=readbuf[6];
        s[1]=readbuf[5];
        s[2]=readbuf[4];
        s[3]=readbuf[3];
        f1=*(float *)s;//采集浓度,低字节前

        s[0]=readbuf[34];
        s[1]=readbuf[33];
        s[2]=readbuf[32];
        s[3]=readbuf[31];
        f2=*(float *)s;//气体系数
        flag='N';
        rtd=f1*f2;
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//声级计
void myAPI::Protocol_3(int port,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf="AWA0";

    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    QString str="([0-9|.]+)dBA,";
    QRegExp Ex;
    Ex.setPattern(str);
    if(Ex.indexIn(readbuf) != -1){
        rtd=Ex.cap(1).toFloat();
        flag='N';      
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//风速
void myAPI::Protocol_4(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    int check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
    if(readbuf.length()>=9){
        if(myHelper::CRC16_Modbus(readbuf.data(),7)==(readbuf[7]+readbuf[8]*256))
        {
            rtd=(readbuf[3]*256+readbuf[4])*0.1;
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//风向
void myAPI::Protocol_5(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    int check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
    if(readbuf.length()>=9){
        if(myHelper::CRC16_Modbus(readbuf.data(),7)==(readbuf[7]+readbuf[8]*256))
        {
            rtd=readbuf[5]*256+readbuf[6];
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//ES-642浓度
void myAPI::Protocol_6(int port,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    int  tt=0;
    QByteArray temp;
    QByteArray readbuf;
    do{
        temp=myCom[port]->readAll();
        readbuf+=temp;
        if(temp.size()==0)tt++;
        else tt=0;
        if(tt>COM[port].Timeout/20)break;
        usleep(20000);
    }while(!readbuf.endsWith("\r\n"));

    if(tt<=COM[port].Timeout/20&&readbuf.size()>0){
        qDebug()<<QString("COM%1 received:%2").arg(port+2).arg(readbuf.data());
        int sum_r=readbuf.right(7).left(5).toInt();
        int sum_c=myHelper::SUM(readbuf.data(),readbuf.length()-8);
        if(sum_r==sum_c){
            rtd=readbuf.split(',')[0].toDouble();
            flag='N';
        }        
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}
//ES-642温度
void myAPI::Protocol_7(int port,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    int  tt=0;
    QByteArray temp;
    QByteArray readbuf;

    do{
        temp=myCom[port]->readAll();
        readbuf+=temp;
        if(temp.size()==0)tt++;
        else tt=0;
        if(tt>COM[port].Timeout/20)break;
        usleep(20000);
    }while(!readbuf.endsWith("\r\n"));

    if(tt<=COM[port].Timeout/20&&readbuf.size()>0){
        qDebug()<<QString("COM%1 received:%2").arg(port+2).arg(readbuf.data());
        int sum_r=readbuf.right(7).left(5).toInt();
        int sum_c=myHelper::SUM(readbuf.data(),readbuf.length()-8);
        if(sum_r==sum_c){
            rtd=readbuf.split(',')[2].toDouble();
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//ES-642湿度
void myAPI::Protocol_8(int port,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    int tt=0;
    QByteArray temp;
    QByteArray readbuf;
    do{
        temp=myCom[port]->readAll();
        readbuf+=temp;
        if(temp.size()==0)tt++;
        else tt=0;
        if(tt>COM[port].Timeout/20)break;
        usleep(20000);
    }while(!readbuf.endsWith("\r\n"));

    if(tt<=COM[port].Timeout/20&&readbuf.size()>0){
        qDebug()<<QString("COM%1 received:%2").arg(port+2).arg(readbuf.data());
        int sum_r=readbuf.right(7).left(5).toInt();
        int sum_c=myHelper::SUM(readbuf.data(),readbuf.length()-8);
        if(sum_r==sum_c){
            rtd=readbuf.split(',')[3].toDouble();
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);

}

//ES-642气压
void myAPI::Protocol_9(int port,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    int  tt=0;
    QByteArray temp;
    QByteArray readbuf;
    do{
        temp=myCom[port]->readAll();
        readbuf+=temp;
        if(temp.size()==0)tt++;
        else tt=0;
        if(tt>COM[port].Timeout/20)break;
        usleep(20000);
    }while(!readbuf.endsWith("\r\n"));

    if(tt<=COM[port].Timeout/20&&readbuf.size()>0){
        qDebug()<<QString("COM%1 received:%2").arg(port+2).arg(readbuf.data());
        int sum_r=readbuf.right(7).left(5).toInt();
        int sum_c=myHelper::SUM(readbuf.data(),readbuf.length()-8);
        if(sum_r==sum_c){
            rtd=readbuf.split(',')[4].toDouble();
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//经度
void myAPI::Protocol_10(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf=QString("##Add=%1;FC=1;CRC=").arg(Address).toAscii();
    QByteArray ch;
    ch.resize(4);

    int len=sendbuf.size();
    int crc=myHelper::CRC16_GB212(&sendbuf.data()[2],len-2);
    sprintf(ch.data(),"%.4X",crc);
    sendbuf.append(ch+"\r\n");
    myCom[port]->write(sendbuf);   
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    if(readbuf.endsWith("\r\n")){
        QRegExp Ex;
        Ex.setPattern("Lot=([0-9|.]+)");
        if(Ex.indexIn(readbuf) != -1){
            double f1=myHelper::Str_To_Double(Ex.cap(1).toLatin1().data());
            int f2=(int)(f1/100);
            rtd=f2+(f1-f2*100)/60;
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//纬度
void myAPI::Protocol_11(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf=QString("##Add=%1;FC=1;CRC=").arg(Address).toAscii();
    QByteArray ch;
    ch.resize(4);

    int len=sendbuf.size();
    int crc=myHelper::CRC16_GB212(&sendbuf.data()[2],len-2);
    sprintf(ch.data(),"%.4X",crc);
    sendbuf.append(ch+"\r\n");
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    if(readbuf.endsWith("\r\n")){
        QRegExp Ex;
        Ex.setPattern("Lat=([0-9|.]+)");
        if(Ex.indexIn(readbuf) != -1){
            double f1=myHelper::Str_To_Double(Ex.cap(1).toLatin1().data());
            int f2=(int)(f1/100);
            rtd=f2+(f1-f2*100)/60;
            flag='N';            
        }      
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//瑾熙流量计
void myAPI::Protocol_12(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    double total=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    int check;
    int len;
    QByteArray ch;
    ch.resize(4);
    char s[4];

    sendbuf.resize(8);

    sendbuf[0]=Address;//读取瞬时流量
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    sendbuf[6]=0xC4;
    sendbuf[7]=0x0B;
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
    if(readbuf.length()>=9){
        len=readbuf.length();
        check = myHelper::CRC16_Modbus(&readbuf.data()[len-9],7);
        qDebug()<<QString::number(check,16).toUpper();
        if((char)(check)==readbuf[len-2] && (char)(check>>8)==readbuf[len-1])
        {
            s[0]=readbuf[len-3];
            s[1]=readbuf[len-4];
            s[2]=readbuf[len-5];
            s[3]=readbuf[len-6];
            rtd=*(float *)s;
            flag='N';
        }
    }

    sendbuf[0]=Address;//读取正向总量
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
    if(readbuf.length()>=9){
        len=readbuf.length();
        check = myHelper::CRC16_Modbus(&readbuf.data()[len-9],7);
        qDebug()<<QString::number(check,16).toUpper();
        if((char)(check)==readbuf[len-2] && (char)(check>>8)==readbuf[len-1])
        {
            total=readbuf[len-6]*16777216+readbuf[len-5]*65536+readbuf[len-4]*256+readbuf[len-3];
        }
    }
//    qDebug()<<QString("Rtd:%1 Total:%2").arg(rtd).arg(total);
    CacheDataProc(rtd,total,flag,Dec,Name,Code,Unit);
}

//光华流量计
void myAPI::Protocol_13(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    double total=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
//    QByteArray ch;
//    ch.resize(4);
    int d0,d1,d2,d3,d4;
    usleep(1000000);
    sendbuf.resize(4);
    sendbuf[0]=0x2A;//读取正向总量
    sendbuf[1]=Address;
    sendbuf[2]=0x04;
    sendbuf[3]=0x2E;
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
    if(readbuf.length()>=10&&0xAA==readbuf[9]){
        d0 = myHelper::to_natural_binary(readbuf[2]);
        d1 = myHelper::to_natural_binary(readbuf[3]);
        d2 = myHelper::to_natural_binary(readbuf[4]);
        d3 = myHelper::to_natural_binary(readbuf[5]);
        d4 = myHelper::to_natural_binary(readbuf[6]);
        total=100000000*d4+1000000*d3+10000*d2+100*d1+d0;
    }
    usleep(1000000);
    sendbuf[0]=0x2A;//读取瞬时流量
    sendbuf[1]=Address;
    sendbuf[2]=0x00;
    sendbuf[3]=0x2E;
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
    if(readbuf.length()>=10&&0xAA==readbuf[9]){
        d0 = myHelper::to_natural_binary(readbuf[2]);
        d1 = myHelper::to_natural_binary(readbuf[3]);
        d2 = myHelper::to_natural_binary(readbuf[4]);
        rtd = 10000*d2+100*d1+d0;
        rtd = rtd*pow(10,readbuf[5]-5);//以10为底的temp次幂
        flag='N';
        CacheDataProc(rtd,total,flag,Dec,Name,Code,Unit);
    }
    usleep(1000000);

//    qDebug()<<QString("Rtd:%1 Total:%2").arg(rtd).arg(total);
//    double total=0;
//    QString flag="D";
//    QByteArray readbuf;
//    QByteArray sendbuf;
//    QByteArray ch;
//    ch.resize(4);
//    int d0,d1,d2,d3,d4;

//    sendbuf.resize(4);
//    usleep(1000000);
//    sendbuf[0]=0x2A;//读取正向总量
//    sendbuf[1]=Address;
//    sendbuf[2]=0x04;
//    sendbuf[3]=0x2E;
//    myCom[port]->write(sendbuf);
//    usleep(COM[port].Timeout*1000);
//    readbuf=myCom[port]->readAll();
//    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
//    if(readbuf.length()>=10&&0xAA==readbuf[9]){
//        d0 = myHelper::to_natural_binary(readbuf[2]);
//        d1 = myHelper::to_natural_binary(readbuf[3]);
//        d2 = myHelper::to_natural_binary(readbuf[4]);
//        d3 = myHelper::to_natural_binary(readbuf[5]);
//        d4 = myHelper::to_natural_binary(readbuf[6]);
//        total=100000000*d4+1000000*d3+10000*d2+100*d1+d0;
//    }

//    usleep(1000000);
//    sendbuf[0]=0x2A;//读取瞬时流量
//    sendbuf[1]=Address;
//    sendbuf[2]=0x00;
//    sendbuf[3]=0x2E;
//    myCom[port]->write(sendbuf);
//    usleep(COM[port].Timeout*1000);
//    readbuf=myCom[port]->readAll();
//    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
//    if(readbuf.length()>=10&&0xAA==readbuf[9]){
//        d0 = myHelper::to_natural_binary(readbuf[2]);
//        d1 = myHelper::to_natural_binary(readbuf[3]);
//        d2 = myHelper::to_natural_binary(readbuf[4]);
//        rtd = 10000*d2+100*d1+d0;
//        rtd = rtd*pow(10,readbuf[5]-5);//以10为底的temp次幂
//        flag="N";
//    }
//   qDebug()<<QString("Rtd:%1 Total:%2").arg(rtd).arg(total);
//    CacheDataProc(rtd,total,flag,Dec,Name,Code,Unit);
//    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);


//    usleep(1000000);
}

//TOC-4100CJ
void myAPI::Protocol_14(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QString rtd_tmp;
    QByteArray readbuf;
    QByteArray sendbuf=QString("##000R%1***").arg(Address).toAscii();

    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    if(readbuf.startsWith("&&&")&&readbuf.endsWith("***")&&0x30==readbuf[8]){
        rtd_tmp=readbuf.mid(22,6);
        rtd=rtd_tmp.toDouble();
        flag='N';
        CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
    }
}

//承德流量计
void myAPI::Protocol_15(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    double total=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    int check=0;
    char s[4];

    sendbuf.resize(14);
    sendbuf[0]=0x68;
    sendbuf[1]=(char)(Address);
    sendbuf[2]=(char)(Address>>8);
    sendbuf[3]=(char)(Address>>16);
    sendbuf[4]=(char)(Address>>24);
    sendbuf[5]=0x00;
    sendbuf[6]=0x00;
    sendbuf[7]=0x68;
    sendbuf[8]=0x01;
    sendbuf[9]=0x02;
    sendbuf[10]=0x18;
    sendbuf[11]=0xC0;
    check=0;
    for(int j=0;j<12;j++)
    {
        check+=sendbuf.data()[j];
    }
    sendbuf[12]=(uchar)check;
    sendbuf[13]=0x16;
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
    if(readbuf.endsWith(0x16)&&readbuf.startsWith(0x68)){
        s[0]=readbuf[12];
        s[1]=readbuf[13];
        s[2]=readbuf[14];
        s[3]=readbuf[15];
        rtd=*(float *)s;
        flag='N';
    }

    sendbuf[0]=0x68;
    sendbuf[1]=(char)(Address);
    sendbuf[2]=(char)(Address>>8);
    sendbuf[3]=(char)(Address>>16);
    sendbuf[4]=(char)(Address>>24);
    sendbuf[5]=0x00;
    sendbuf[6]=0x00;
    sendbuf[7]=0x68;
    sendbuf[8]=0x01;
    sendbuf[9]=0x02;
    sendbuf[10]=0x03;
    sendbuf[11]=0xC0;
    check=0;
    for(int j=0;j<12;j++)
    {
        check+=sendbuf.data()[j];
    }
    sendbuf[12]=(uchar)check;
    sendbuf[13]=0x16;
    myCom[port]->write(sendbuf);//读取累积流量
    usleep(COM[port].Timeout*1000);
    readbuf = myCom[port]->readAll();
    if(readbuf.endsWith(0x16)&&readbuf.startsWith(0x68))
    {
        qDebug()<<readbuf.toHex();

        total=  myHelper::to_natural_binary(readbuf[19])*100000000000LL+
                    myHelper::to_natural_binary(readbuf[18])*1000000000+
                    myHelper::to_natural_binary(readbuf[17])*10000000+
                    myHelper::to_natural_binary(readbuf[16])*100000+
                    myHelper::to_natural_binary(readbuf[15])*1000+
                    myHelper::to_natural_binary(readbuf[14])*10+
                    myHelper::to_natural_binary(readbuf[13])*0.1+
                    myHelper::to_natural_binary(readbuf[12])*0.001;
    }
    CacheDataProc(rtd,total,flag,Dec,Name,Code,Unit);
}


//SJFC-200苏净粉尘仪
void myAPI::Protocol_16(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    int32_t rtd_tmp=0;
    QString flag="D";

    QByteArray readbuf;
    QByteArray sendbuf;

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x9C;
    sendbuf[3]=0x46;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    int check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    if(readbuf.length()>=9){
        qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
        if(Address==readbuf[0])
        {
                check = myHelper::CRC16_Modbus(readbuf.data(),7);
                if((readbuf[7]==(char)(check&0xff))&&(readbuf[8]==(char)(check>>8)))
                {
                    rtd_tmp=readbuf[3];
                    rtd_tmp=rtd_tmp<<8;
                    rtd_tmp+=readbuf[4];
                    rtd_tmp=rtd_tmp<<8;
                    rtd_tmp+=readbuf[5];
                    rtd_tmp=rtd_tmp<<8;
                    rtd_tmp+=readbuf[6];
                    rtd=(float)rtd_tmp/10;                     //单位：ug/m3
                    flag='N';
                }
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//天泽温度
void myAPI::Protocol_17(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf=QString("##Add=%1;FC=1;CRC=").arg(Address).toAscii();
    QByteArray ch;
    ch.resize(4);

    int len=sendbuf.size();
    int crc=myHelper::CRC16_GB212(&sendbuf.data()[2],len-2);
    sprintf(ch.data(),"%.4X",crc);
    sendbuf.append(ch+"\r\n");
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    if(readbuf.endsWith("\r\n")){
        QRegExp Ex;                             //
        Ex.setPattern("Temp=([0-9]+)");
        if(Ex.indexIn(readbuf) != -1){
            double f1=myHelper::Str_To_Double(Ex.cap(1).toLatin1().data());
            rtd=f1/100;
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}
//天泽湿度
void myAPI::Protocol_18(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf=QString("##Add=%1;FC=1;CRC=").arg(Address).toAscii();
    QByteArray ch;
    ch.resize(4);

    int len=sendbuf.size();
    int crc=myHelper::CRC16_GB212(&sendbuf.data()[2],len-2);
    sprintf(ch.data(),"%.4X",crc);
    sendbuf.append(ch+"\r\n");
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    if(readbuf.endsWith("\r\n")){
        QRegExp Ex;                             //
        Ex.setPattern("Hum=([0-9]+)");
        if(Ex.indexIn(readbuf) != -1){
            double f1=myHelper::Str_To_Double(Ex.cap(1).toLatin1().data());
            rtd=f1/100;
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}
//天泽气压
void myAPI::Protocol_19(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf=QString("##Add=%1;FC=1;CRC=").arg(Address).toAscii();
    QByteArray ch;
    ch.resize(4);

    int len=sendbuf.size();
    int crc=myHelper::CRC16_GB212(&sendbuf.data()[2],len-2);
    sprintf(ch.data(),"%.4X",crc);
    sendbuf.append(ch+"\r\n");
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    if(readbuf.endsWith("\r\n")){
        QRegExp Ex;                             //
        Ex.setPattern("Prs=([0-9]+)");
        if(Ex.indexIn(readbuf) != -1){
            double f1=myHelper::Str_To_Double(Ex.cap(1).toLatin1().data());
            rtd=f1/100;
            flag='N';
        }
    }
    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//光华流量计-总量
void myAPI::Protocol_20(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    QByteArray ch;
    ch.resize(4);
    int d0,d1,d2,d3,d4;

    sendbuf.resize(4);

//    sendbuf[0]=0x2A;//读取瞬时流量
//    sendbuf[1]=Address;
//    sendbuf[2]=0x00;
//    sendbuf[3]=0x2E;
//    myCom[port]->write(sendbuf);
//    usleep(COM[port].Timeout*1000);
//    readbuf=myCom[port]->readAll();
//    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
//    if(readbuf.length()>=10&&0xAA==readbuf[9]){
//        d0 = myHelper::to_natural_binary(readbuf[2]);
//        d1 = myHelper::to_natural_binary(readbuf[3]);
//        d2 = myHelper::to_natural_binary(readbuf[4]);
//        rtd = 10000*d2+100*d1+d0;
//        rtd = rtd*pow(10,readbuf[5]-5);//以10为底的temp次幂
//        flag='N';
//    }
    usleep(1000000);
    sendbuf[0]=0x2A;//读取正向总量
    sendbuf[1]=Address;
    sendbuf[2]=0x04;
    sendbuf[3]=0x2E;
    myCom[port]->readAll();
    myCom[port]->write(sendbuf);
    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex();
    if(readbuf.length()>=13&&Address==readbuf[0]){
        d0 = myHelper::to_natural_binary(readbuf[2]);
        d1 = myHelper::to_natural_binary(readbuf[3]);
        d2 = myHelper::to_natural_binary(readbuf[4]);
        d3 = myHelper::to_natural_binary(readbuf[5]);
        d4 = myHelper::to_natural_binary(readbuf[6]);
        rtd=100000000*d4+1000000*d3+10000*d2+100*d1+d0;
        flag='N';
        CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
    }
    usleep(1000000);
//    qDebug()<<QString("Rtd:%1 Total:%2").arg(rtd).arg(total);
//    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}
//PH-P206
void myAPI::Protocol_21(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    int check=0;
   volatile char s[4];

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x04;
    sendbuf[2]=0x00;
    sendbuf[3]=0x02;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    qDebug()<<QString("COM%1 send:").arg(port+2)<<sendbuf.toHex().toUpper();
    myCom[port]->write(sendbuf);
    sleep(1);
//    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
    if(readbuf.length()>=9){
        if(Address==readbuf[0])
        {
                check = myHelper::CRC16_Modbus(readbuf.data(),7);
                if((readbuf[7]==(char)(check&0xff))&&(readbuf[8]==(char)(check>>8)))
                {
                    s[0]=readbuf[4];
                    s[1]=readbuf[3];
                    s[2]=readbuf[6];
                    s[3]=readbuf[5];
                    rtd=*(float *)s;        //瞬时PH
                    flag='N';
                }
        }
    }

    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//明渠流量计
void myAPI::Protocol_22(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    double total=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    int check=0;
    char s[4];

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x08;
    sendbuf[4]=0x00;
    sendbuf[5]=0x04;
    check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    qDebug()<<QString("COM%1 send:").arg(port+2)<<sendbuf.toHex().toUpper();
    myCom[port]->write(sendbuf);
    sleep(1);
//    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
    if(readbuf.length()>=13){
        if(Address==readbuf[0])
        {
                check = myHelper::CRC16_Modbus(readbuf.data(),11);
                if((readbuf[11]==(char)(check&0xff))&&(readbuf[12]==(char)(check>>8)))
                {
                    s[0]=readbuf[6];
                    s[1]=readbuf[5];
                    s[2]=readbuf[4];
                    s[3]=readbuf[3];
                    rtd=*(float *)s;        //瞬时流量单位M3/H
                    s[0]=readbuf[10];
                    s[1]=readbuf[9];
                    s[2]=readbuf[8];
                    s[3]=readbuf[7];
                    total=*(float *)s;        //累计流量单位M3
                    flag='N';
                }
        }
    }

    CacheDataProc(rtd,total,flag,Dec,Name,Code,Unit);

}
//C10电导率
void myAPI::Protocol_23(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    char check=0;
    char ch_temp;
    volatile char s[4];
    sendbuf.resize(14);
    sendbuf[0]=0x40;
    sendbuf[1]=((Address >> 4) & 0x0f)+0x30;
    sendbuf[2]=(Address&0x0f)+0x30;
    sendbuf[3]='C';
    sendbuf[4]='8';
    sendbuf[5]='0';
    sendbuf[6]='0';
    sendbuf[7]='0';
    sendbuf[8]='0';
    sendbuf[9]='0';
    sendbuf[10]='8';
    check=myHelper::XORValid(&sendbuf.data()[1],10);
    ch_temp = (check  >> 4) & 0x0F;  //取高位数；
    if (ch_temp < 10)                       //低于10的数
        ch_temp = ch_temp  +  '0';
    else
        ch_temp = (ch_temp - 10 ) +  'A';   //不低于10的16进制数，如：A、B、C、D、E、F
    sendbuf[11]=ch_temp;
    ch_temp = check & 0x0F;  //取低位数；
    if (ch_temp < 10) ch_temp = ch_temp  +  '0';
    else ch_temp = (ch_temp - 10 )+  'A';
    sendbuf[12]=ch_temp;
    sendbuf[13]=0x0D;
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 send:").arg(port+2)<<sendbuf;
    myCom[port]->write(sendbuf);
    sleep(1);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
    if(readbuf.length()>=24&&0x40==readbuf[0]){
        if(Address==(readbuf[1]-0x30)*16+readbuf[2]-0x30)
        {
            check = myHelper::XORValid(&readbuf.data()[1],readbuf.length()-4);
            ch_temp = (check  >> 4) & 0x0F;  //取高位数；
            if (ch_temp < 10)                       //低于10的数
                ch_temp = ch_temp  +  '0';
            else
                ch_temp = (ch_temp - 10 ) +  'A';   //不低于10的16进制数，如：A、B、C、D、E、F
            if(readbuf[readbuf.length()-3]!=ch_temp) return;
            ch_temp = check & 0x0F;  //取低位数；
            if (ch_temp < 10) ch_temp = ch_temp  +  '0';
            else ch_temp = (ch_temp - 10 )+  'A';
            if(readbuf[readbuf.length()-2]!=ch_temp) return;
//            qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf;
            s[0]=myHelper::HexStrValue(readbuf[11],readbuf[12]);
            s[1]=myHelper::HexStrValue(readbuf[9],readbuf[10]);
            s[2]=myHelper::HexStrValue(readbuf[7],readbuf[8]);
            s[3]=myHelper::HexStrValue(readbuf[5],readbuf[6]);
            rtd=*(float *)s;        //瞬时电导率
//            qDebug()<<QString("rtd=%1").arg(rtd);

            flag='N';
        }
    }

    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
}

//微兰COD
void myAPI::Protocol_24(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    int check=0;
   volatile char s[4];

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    qDebug()<<QString("COM%1 send:").arg(port+2)<<sendbuf.toHex().toUpper();
    myCom[port]->write(sendbuf);
    sleep(1);
//    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
    if(readbuf.length()>=9){
        if(Address==readbuf[0])
        {
                check = myHelper::CRC16_Modbus(readbuf.data(),7);
                if((readbuf[7]==(char)(check&0xff))&&(readbuf[8]==(char)(check>>8)))
                {
                    s[0]=readbuf[4];
                    s[1]=readbuf[3];
                    s[2]=readbuf[6];
                    s[3]=readbuf[5];
                    rtd=*(float *)s;        //瞬时COD
                    flag='N';
                }
        }
    }

    CacheDataProc(rtd,0,flag,Dec,Name,Code,Unit);
    //添加取样指令
}

//雨水刷卡设备
void myAPI::Protocol_25()
{
   ;
}

//哈希COD
void myAPI::Protocol_26(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    double total=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    int check=0;
    char s[4];

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x06;
    check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    qDebug()<<QString("COM%1 send:").arg(port+2)<<sendbuf.toHex().toUpper();
    myCom[port]->write(sendbuf);
    sleep(1);
//    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
    if(readbuf.length()>=17){
        if(Address==readbuf[0])
        {
                check = myHelper::CRC16_Modbus(readbuf.data(),readbuf.length()-2);
                if((readbuf[readbuf.length()-2]==(char)(check&0xff))&&(readbuf[readbuf.length()-1]==(char)(check>>8)))
                {
                    s[0]=readbuf[4];
                    s[1]=readbuf[3];
                    s[2]=readbuf[6];
                    s[3]=readbuf[5];
                    rtd=*(float *)s;        //COD单位
                    flag='N';
                }
        }
    }

    CacheDataProc(rtd,total,flag,Dec,Name,Code,Unit);
}

//哈希氨氮
void myAPI::Protocol_27(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
{
    double rtd=0;
    double total=0;
    QString flag="D";
    QByteArray readbuf;
    QByteArray sendbuf;
    int check=0;
    char s[4];

    sendbuf.resize(8);
    sendbuf[0]=Address;
    sendbuf[1]=0x03;
    sendbuf[2]=0x00;
    sendbuf[3]=0x00;
    sendbuf[4]=0x00;
    sendbuf[5]=0x02;
    check = myHelper::CRC16_Modbus(sendbuf.data(),6);
    sendbuf[6]=(char)(check);
    sendbuf[7]=(char)(check>>8);
    qDebug()<<QString("COM%1 send:").arg(port+2)<<sendbuf.toHex().toUpper();
    myCom[port]->write(sendbuf);
    sleep(1);
//    usleep(COM[port].Timeout*1000);
    readbuf=myCom[port]->readAll();
    qDebug()<<QString("COM%1 received:").arg(port+2)<<readbuf.toHex().toUpper();
    if(readbuf.length()>=9){
        if(Address==readbuf[0])
        {
                check = myHelper::CRC16_Modbus(readbuf.data(),readbuf.length()-2);
                if((readbuf[readbuf.length()-2]==(char)(check&0xff))&&(readbuf[readbuf.length()-1]==(char)(check>>8)))
                {
                    s[0]=readbuf[4];
                    s[1]=readbuf[3];
                    s[2]=readbuf[6];
                    s[3]=readbuf[5];
                    rtd=*(float *)s;        //NH3-N单位
                    flag='N';
                }
        }
    }

    CacheDataProc(rtd,total,flag,Dec,Name,Code,Unit);
}

////KS-3200采样仪
//void myAPI::Protocol_28(int port,int Address,int Dec,QString Name,QString Code,QString Unit)
//{
//   ;
//}



//与设备进行通讯
void myAPI::MessageFromCom(int port)   //内存数据库操作
{
    QString Code;
    QString Name;
    QString  Unit;
    int Address;
    int Decimals;

    QString sql=QString("select * from [ParaInfo] where [UseChannel]='COM%1'").arg(port+2);
    QSqlQuery query;
    query.exec(sql);
    while(query.next())
    {
        Name=query.value(0).toString();
        Code=query.value(1).toString();
        Unit=query.value(2).toString();
        Address=query.value(4).toInt();
        Decimals=query.value(15).toInt();

        switch (query.value(5).toInt())//通讯协议
        {
        case 1://天泽VOCs
                Protocol_2(port,Address,Decimals,Name,Code,Unit);
                break;
        case 2://声级计
                Protocol_3(port,Decimals,Name,Code,Unit);
                break;
        case 3://风速
                 Protocol_4(port,Address,Decimals,Name,Code,Unit);
                break;
        case 4://风向
                Protocol_5(port,Address,Decimals,Name,Code,Unit);
                break;
        case 5://ES-642浓度
                Protocol_6(port,Decimals,Name,Code,Unit);
                break;
        case 6://ES-642温度
                Protocol_7(port,Decimals,Name,Code,Unit);
                break;
        case 7://ES-642湿度
                Protocol_8(port,Decimals,Name,Code,Unit);
                break;
        case 8://气压
                Protocol_9(port,Decimals,Name,Code,Unit);
                break;
        case 9://经度
                Protocol_10(port,Address,Decimals,Name,Code,Unit);
                break;
        case 10://纬度
                Protocol_11(port,Address,Decimals,Name,Code,Unit);
                break;
        case 11://瑾熙流量计
                Protocol_12(port,Address,Decimals,Name,Code,Unit);
                break;
        case 12://光华流量计
                Protocol_13(port,Address,Decimals,Name,Code,Unit);
                break;
        case 13://TOC-4100CJ
                Protocol_14(port,Address,Decimals,Name,Code,Unit);
                break;
        case 14://承德流量计
                Protocol_15(port,Address,Decimals,Name,Code,Unit);
                break;
        case 15://SJFC-200
                Protocol_16(port,Address,Decimals,Name,Code,Unit);
                break;
        case 16://天泽温度
                Protocol_17(port,Address,Decimals,Name,Code,Unit);
                break;
        case 17://天泽湿度
                Protocol_18(port,Address,Decimals,Name,Code,Unit);
                break;
        case 18://天泽气压
                Protocol_19(port,Address,Decimals,Name,Code,Unit);
                break;
        case 19://光华流量计-总量
                Protocol_20(port,Address,Decimals,Name,Code,Unit);
            break;
        case 20://PH-P206
            Protocol_21(port,Address,Decimals,Name,Code,Unit);
        break;
        case 21://明渠流量计
            Protocol_22(port,Address,Decimals,Name,Code,Unit);
        break;
        case 22://电导率
            Protocol_23(port,Address,Decimals,Name,Code,Unit);
        break;            
        case 23://微兰COD
            Protocol_24(port,Address,Decimals,Name,Code,Unit);
        break;
        case 24://雨水刷卡设备
            Protocol_25();
        break;
        case 25://哈希COD
            Protocol_26(port,Address,Decimals,Name,Code,Unit);
        break;
        case 26://哈希氨氮
            Protocol_27(port,Address,Decimals,Name,Code,Unit);
        break;
//        case 27://KS-3200采样仪
//            Protocol_28(port,Address,Decimals,Name,Code,Unit);
//        break;
       default: break;


        }
        usleep(COM[port].Interval*1000);
    }


}



