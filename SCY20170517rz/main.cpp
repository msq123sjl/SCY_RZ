#include "frmmain.h"
#include <QTextCodec>
#include <QTranslator>
#include <QDesktopWidget>
#include "api/myapp.h"
#include "api/myhelper.h"
#include "api/myapi.h"
#include <QFont>
#if QT_VERSION >= 0x050000
#include <QApplication>
#else
#include <QtGui/QApplication>
#endif
#include "frminput.h"
#include "frmnum.h"
#include "myscreensaver.h"
extern QSqlDatabase        DbConn;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont font("SIMSUN",9,QFont::Normal);           //设置字体
    a.setFont(font);
    MyScreenSaver screensaver;
    a.installEventFilter(&screensaver);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");      //中文字体编码方式
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);

//    a.setApplicationName("SCY");        //设置应用程序名称
//    a.setApplicationVersion("V1.0");    //设置应用程序版本
//     myHelper::SetUTF8Code();            //设置程序编码为UTF-8
//     myHelper::SetStyle();                    //设置应用程序样式

    //赋值当前应用程序路径和桌面宽度高度
    myApp::AppPath=QApplication::applicationDirPath()+"/";
    myApp::DeskWidth=qApp->desktop()->availableGeometry().width();
    myApp::DeskHeigth=qApp->desktop()->availableGeometry().height();

    //判断当前数据库文件是否存在(如果数据库打开失败则终止应用程序)
    if (!myHelper::FileIsExist(myApp::AppPath+"db_scy.db")){
//       if (!myHelper::FileIsExist("/mnt/sdcard/db_scy.db")){

        myHelper::ShowMessageBoxError("数据库文件不存在,程序将自动关闭！");
        return 1;
    }

    QSqlDatabase DbConn;                //
    DbConn=QSqlDatabase::addDatabase("QSQLITE");
    DbConn.setDatabaseName(myApp::AppPath+"db_scy.db");
//    DbConn.setDatabaseName("/mnt/sdcard/db_scy.db");
    //创建数据库连接并打开(如果数据库打开失败则终止应用程序)
    if (!DbConn.open()){
        myHelper::ShowMessageBoxError("打开数据库失败,程序将自动关闭！");
        return 1;
    }

    QSqlDatabase memorydb;
    memorydb=QSqlDatabase::addDatabase("QSQLITE","memory");
    //如果要连接多个数据库，必须指定不同的连接名字，否则会覆盖上面的默认连接
    memorydb.setDatabaseName(":memory:");
    if(!memorydb.open()){
        myHelper::ShowMessageBoxError("打开内部数据库失败，程序将自动关闭！");
    }
    else
    {
        QSqlQuery query_memory(QSqlDatabase::database("memory",true));
        query_memory.exec("create table CacheRtd(GetTime DATETIME(20),Name NVARCHAR(20),Code NVARCHAR(10),Rtd NVARCHAR(20),Total NVARCHAR(20),Flag NVARCHAR(5),ErrorNums NVARCHAR(2));");
    }
    //程序加载时先加载所有配置信息
    myApp::ReadConfig();
//    if(!myHelper::MD5_EncryptDecrypt(myApp::Key))//防止应用拷贝,在其他设备上无法运行
//    {
//        myHelper::ShowMessageBoxError("本机无法正常运行！");
//        return 1;
//    }

    frmMain w;
    w.showFullScreen();


    //在main函数中就加载输入法面板,保证整个应用程序可以使用输入法
    //以下方法打开中文输入法
    frmInput::Instance()->Init("control", "black", 10, 10);
    //以下方法打开数字键盘
    //frmNum::Instance()->Init("black", 10);
    myAPI *api=new myAPI;
    api->AddEventInfoUser("系统启动运行");
    delete api;
    int ret=0;
    ret=a.exec();
    DbConn.close();
    return ret;

}
