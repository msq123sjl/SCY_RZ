#include "frmvalve.h"
#include "ui_frmvalve.h"
#include "api/gpio.h"
#include "api/myhelper.h"
#include "api/myapi.h"

enum Status{IsOpen,IsClose,IsError}Valve_CurrentStatus,Valve_RightStatus;
bool Set_Flag;
int   Valve_Set_Count;

frmValve::frmValve(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmValve)
{
    ui->setupUi(this);

    myHelper::FormInCenter(this,myApp::DeskWidth,myApp::DeskHeigth);//窗体居中显示
    this->InitStyle();
}

frmValve::~frmValve()
{
    delete ui;
}

void frmValve::InitStyle()
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
}

//阀门开启输出使能
void frmValve::Valve_Open_Set()
{
    Valve_Close_Clear();
    SwitchOut_On(6);
    Set_Flag=true;
    Valve_RightStatus=IsOpen;
    Valve_Set_Count=0;
}


//阀门关闭输出使能
void frmValve::Valve_Close_Set()
{
    Valve_Open_Clear();
    SwitchOut_On(7);
    Set_Flag=true;
    Valve_RightStatus=IsClose;
    Valve_Set_Count=0;
}

//阀门开启输出清除
void frmValve::Valve_Open_Clear()
{
    SwitchOut_Off(6);
    Set_Flag=false;
}

//阀门关闭输出清除
void frmValve::Valve_Close_Clear()
{
    SwitchOut_Off(7);
    Set_Flag=false;
}

//手动开阀
void frmValve::on_btn_ValveOpen_clicked()
{
    if (myHelper::ShowMessageBoxQuesion("确定开启阀吗?")==0){
        Valve_Open_Set();
    }
}
//手动关阀
void frmValve::on_btn_ValveClose_clicked()
{
    if (myHelper::ShowMessageBoxQuesion("确定关闭阀吗?")==0){
        Valve_Close_Set();
    }
}

void frmValve::on_btn_Cancel_clicked()
{
    this->close();
}
