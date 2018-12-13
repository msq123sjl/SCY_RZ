#include "frmsampler.h"
#include "ui_frmsampler.h"
#include "api/myapp.h"
#include "api/myhelper.h"

extern Sampler sampler;
extern QextSerialPort *myCom[6];
#define COM_PORT 4
frmSampler::frmSampler(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmSampler)
{
    ui->setupUi(this);
    myHelper::FormInCenter(this,myApp::DeskWidth,myApp::DeskHeigth);//窗体居中显示
    this->InitStyle();
    this->InitForm();
}

frmSampler::~frmSampler()
{
    delete ui;
}

void frmSampler::InitStyle()//此处作用？
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
}

void frmSampler::InitForm()
{
     myApp::ReadConfig();
    ui->comboBox_sampleMode->setCurrentIndex(sampler.SampleMode);                                                                                                                              //采样器型号
    ui->txtSampleFlow->setText(QString::number(sampler.SampleFlow));                                                                                                                                  //采样量
    ui->comboBox_SampleStartBottle->setCurrentIndex(ui->comboBox_SampleStartBottle->findText(QString::number(sampler.SampleStartBottle)));//开始瓶号
    GetSamplerStatus();
}

//定量
bool frmSampler::SampleFixFlow()
{
    QByteArray sendbuf;
    QByteArray readbuf;
    bool return_value=false;
    myApp::ReadConfig();
    if(sampler.SampleStartBottle>24)sampler.SampleStartBottle=24;
    if(sampler.SampleFlow>1000)sampler.SampleFlow=1000;

    if(0==sampler.SampleMode)                  //SBC-6000协议
    {
        sendbuf[0]=0xAA;
        sendbuf[1]=0x34;
        sendbuf[2]=sampler.SampleFlow/10000+0x30;
        sendbuf[3]=sampler.SampleFlow%10000/1000+0x30;
        sendbuf[4]=sampler.SampleFlow%1000/100+0x30;
        sendbuf[5]=sampler.SampleFlow%100/10+0x30;
        sendbuf[6]=sampler.SampleFlow%10+0x30;
        sendbuf[7]=sampler.SampleStartBottle/10+0x30;
        sendbuf[8]=sampler.SampleStartBottle%10+0x30;
        sendbuf[9]=0xBB;
        myCom[COM_PORT-2]->readAll();
        myCom[COM_PORT-2]->write(sendbuf);
        return_value=true;
//        myHelper::Sleep(1000);
//        readbuf=myCom[0]->readAll();
//        if(0xCC==readbuf[0]&&0xDD==readbuf[1]&&0xAA==readbuf[2]&&0xF0==readbuf[3]&&0xBB==readbuf[4]){
//            return true;
//        }else {
//            return false;
//        }
    }
    else if(1==sampler.SampleMode)     //北环科协议
    {
        sendbuf[0]=0x25;
        sendbuf[1]=0x43;
        sendbuf[2]=0X36;
        sendbuf[3]=myHelper::to_BCD(sampler.SampleStartBottle-1);
        sendbuf[4]=myHelper::to_BCD(sampler.SampleFlow/10);
        sendbuf[5]=0x0D;
        sendbuf[6]=(char)(myHelper::SUM(sendbuf.data(),6));
        myCom[COM_PORT-2]->readAll();
        myCom[COM_PORT-2]->write(sendbuf);
        myHelper::Sleep(1000);
        readbuf=myCom[COM_PORT-2]->readAll();
        if(0x25==readbuf[0]&&0x43==readbuf[1]&&0x37==readbuf[2]&&0x79==readbuf[5]){
            return_value=true;
        }else {
            return_value=false;
        }
    }

    QString fileName=myApp::AppPath+"config_web.txt";
    QSettings *set=new QSettings(fileName,QSettings::IniFormat);
    set->beginGroup("WebConfig");
    myApp::SampleCmd=0;
    set->setValue("SampleCmd",myApp::SampleCmd);
    set->endGroup();
    return return_value;
}


//获取采样瓶状态
bool frmSampler::GetSamplerStatus()
{
    int bottle_1=0;
    int bottle_2=0;
    int bottle_3=0;
    int bottle_4=0;
    int bottle_5=0;
    int bottle_6=0;
    int bottle_7=0;
    int bottle_8=0;
    int bottle_9=0;
    int bottle_10=0;
    int bottle_11=0;
    int bottle_12=0;
    int bottle_13=0;
    int bottle_14=0;
    int bottle_15=0;
    int bottle_16=0;
    int bottle_17=0;
    int bottle_18=0;
    int bottle_19=0;
    int bottle_20=0;
    int bottle_21=0;
    int bottle_22=0;
    int bottle_23=0;
    int bottle_24=0;
    QByteArray sendbuf;
    QByteArray readbuf;
    int i=0;
    if(0==sampler.SampleMode)                  //SBC-6000协议
    {
        sendbuf[0]=0xAA;
        sendbuf[1]=0x35;
        sendbuf[2]=0xBB;
        myCom[COM_PORT-2]->readAll();
        myCom[COM_PORT-2]->write(sendbuf);

        myHelper::Sleep(4000);
        readbuf=myCom[COM_PORT-2]->readAll();
        for(i=0;i<readbuf.length();i++)
        {
            if(0xCC==readbuf[i])
                break;
        }
        qDebug()<<QString("COM%1 received:").arg(COM_PORT)<<readbuf.toHex().toUpper();

        if(0xCC==readbuf[i+0]&&0xDD==readbuf[i+1]&&readbuf.length()>48){
             if(0xAA==readbuf[i+2]){
                 bottle_1=readbuf[i+3]*256+readbuf[i+4];
                 bottle_2=readbuf[i+5]*256+readbuf[i+6];
                 bottle_3=readbuf[i+7]*256+readbuf[i+8];
                 bottle_4=readbuf[i+9]*256+readbuf[i+10];
                 bottle_5=readbuf[i+11]*256+readbuf[i+12];
                 bottle_6=readbuf[i+13]*256+readbuf[i+14];
                 bottle_7=readbuf[i+15]*256+readbuf[i+16];
                 bottle_8=readbuf[i+17]*256+readbuf[i+18];
                 bottle_9=readbuf[i+19]*256+readbuf[i+20];
                 bottle_10=readbuf[i+21]*256+readbuf[i+22];
                 bottle_11=readbuf[i+23]*256+readbuf[i+24];
                 bottle_12=readbuf[i+25]*256+readbuf[i+26];
                 bottle_13=readbuf[i+27]*256+readbuf[i+28];
                 bottle_14=readbuf[i+29]*256+readbuf[i+30];
                 bottle_15=readbuf[i+31]*256+readbuf[i+32];
                 bottle_16=readbuf[i+33]*256+readbuf[i+34];
                 bottle_17=readbuf[i+35]*256+readbuf[i+36];
                 bottle_18=readbuf[i+37]*256+readbuf[i+38];
                 bottle_19=readbuf[i+39]*256+readbuf[i+40];
                 bottle_20=readbuf[i+41]*256+readbuf[i+42];
                 bottle_21=readbuf[i+43]*256+readbuf[i+44];
                 bottle_22=readbuf[i+45]*256+readbuf[i+46];
                 bottle_23=readbuf[i+47]*256+readbuf[i+48];
                 bottle_24=readbuf[i+49]*256+readbuf[i+50];
             }
             else{
                 bottle_1=readbuf[i+2]*256+readbuf[i+3];
                 bottle_2=readbuf[i+4]*256+readbuf[i+5];
                 bottle_3=readbuf[i+6]*256+readbuf[i+7];
                 bottle_4=readbuf[i+8]*256+readbuf[i+9];
                 bottle_5=readbuf[i+10]*256+readbuf[i+11];
                 bottle_6=readbuf[i+12]*256+readbuf[i+13];
                 bottle_7=readbuf[i+14]*256+readbuf[i+15];
                 bottle_8=readbuf[i+16]*256+readbuf[i+17];
                 bottle_9=readbuf[i+18]*256+readbuf[i+19];
                 bottle_10=readbuf[i+20]*256+readbuf[i+21];
                 bottle_11=readbuf[i+22]*256+readbuf[i+23];
                 bottle_12=readbuf[i+24]*256+readbuf[i+25];
                 bottle_13=readbuf[i+26]*256+readbuf[i+27];
                 bottle_14=readbuf[i+28]*256+readbuf[i+29];
                 bottle_15=readbuf[i+30]*256+readbuf[i+31];
                 bottle_16=readbuf[i+32]*256+readbuf[i+33];
                 bottle_17=readbuf[i+34]*256+readbuf[i+35];
                 bottle_18=readbuf[i+36]*256+readbuf[i+37];
                 bottle_19=readbuf[i+38]*256+readbuf[i+39];
                 bottle_20=readbuf[i+40]*256+readbuf[i+41];
                 bottle_21=readbuf[i+42]*256+readbuf[i+43];
                 bottle_22=readbuf[i+44]*256+readbuf[i+45];
                 bottle_23=readbuf[i+46]*256+readbuf[i+47];
                 bottle_24=readbuf[i+48]*256+readbuf[i+49];
             }
             QString fileName=myApp::AppPath+"BottleStatus.txt";
             QSettings *set=new QSettings(fileName,QSettings::IniFormat);
             set->beginGroup("BottleStatus");
             set->setValue("bottle_1",bottle_1);
             set->setValue("bottle_2",bottle_2);
             set->setValue("bottle_3",bottle_3);
             set->setValue("bottle_4",bottle_4);
             set->setValue("bottle_5",bottle_5);
             set->setValue("bottle_6",bottle_6);
             set->setValue("bottle_7",bottle_7);
             set->setValue("bottle_8",bottle_8);
             set->setValue("bottle_9",bottle_9);
             set->setValue("bottle_10",bottle_10);
             set->setValue("bottle_11",bottle_11);
             set->setValue("bottle_12",bottle_12);
             set->setValue("bottle_13",bottle_13);
             set->setValue("bottle_14",bottle_14);
             set->setValue("bottle_15",bottle_15);
             set->setValue("bottle_16",bottle_16);
             set->setValue("bottle_17",bottle_17);
             set->setValue("bottle_18",bottle_18);
             set->setValue("bottle_19",bottle_19);
             set->setValue("bottle_20",bottle_20);
             set->setValue("bottle_21",bottle_21);
             set->setValue("bottle_22",bottle_22);
             set->setValue("bottle_23",bottle_23);
             set->setValue("bottle_24",bottle_24);
             set->endGroup();
            return true;
        }
        else return false;
    }
    else return false;


}

void frmSampler::showbottle()
{
    int bottle_1=0;
    int bottle_2=0;
    int bottle_3=0;
    int bottle_4=0;
    int bottle_5=0;
    int bottle_6=0;
    int bottle_7=0;
    int bottle_8=0;
    int bottle_9=0;
    int bottle_10=0;
    int bottle_11=0;
    int bottle_12=0;
    int bottle_13=0;
    int bottle_14=0;
    int bottle_15=0;
    int bottle_16=0;
    int bottle_17=0;
    int bottle_18=0;
    int bottle_19=0;
    int bottle_20=0;
    int bottle_21=0;
    int bottle_22=0;
    int bottle_23=0;
    int bottle_24=0;
    QString fileName=myApp::AppPath+"BottleStatus.txt";
    QSettings *set=new QSettings(fileName,QSettings::IniFormat);
    set->beginGroup("BottleStatus");
    bottle_1=set->value("bottle_1").toInt();
    bottle_2=set->value("bottle_2").toInt();
    bottle_3=set->value("bottle_3").toInt();
    bottle_4=set->value("bottle_4").toInt();
    bottle_5=set->value("bottle_5").toInt();
    bottle_6=set->value("bottle_6").toInt();
    bottle_7=set->value("bottle_7").toInt();
    bottle_8=set->value("bottle_8").toInt();
    bottle_9=set->value("bottle_9").toInt();
    bottle_10=set->value("bottle_10").toInt();
    bottle_11=set->value("bottle_11").toInt();
    bottle_12=set->value("bottle_12").toInt();
    bottle_13=set->value("bottle_13").toInt();
    bottle_14=set->value("bottle_14").toInt();
    bottle_15=set->value("bottle_15").toInt();
    bottle_16=set->value("bottle_16").toInt();
    bottle_17=set->value("bottle_17").toInt();
    bottle_18=set->value("bottle_18").toInt();
    bottle_19=set->value("bottle_19").toInt();
    bottle_20=set->value("bottle_20").toInt();
    bottle_21=set->value("bottle_21").toInt();
    bottle_22=set->value("bottle_22").toInt();
    bottle_23=set->value("bottle_23").toInt();
    bottle_24=set->value("bottle_24").toInt();
    set->endGroup();

    ui->bottle_status_1->setText(QString::number(bottle_1,10));
    ui->bottle_status_2->setText(QString::number(bottle_2,10));
    ui->bottle_status_3->setText(QString::number(bottle_3,10));
    ui->bottle_status_4->setText(QString::number(bottle_4,10));
    ui->bottle_status_5->setText(QString::number(bottle_5,10));
    ui->bottle_status_6->setText(QString::number(bottle_6,10));
    ui->bottle_status_7->setText(QString::number(bottle_7,10));
    ui->bottle_status_8->setText(QString::number(bottle_8,10));
    ui->bottle_status_9->setText(QString::number(bottle_9,10));
    ui->bottle_status_10->setText(QString::number(bottle_10,10));
    ui->bottle_status_11->setText(QString::number(bottle_11,10));
    ui->bottle_status_12->setText(QString::number(bottle_12,10));
    ui->bottle_status_13->setText(QString::number(bottle_13,10));
    ui->bottle_status_14->setText(QString::number(bottle_14,10));
    ui->bottle_status_15->setText(QString::number(bottle_15,10));
    ui->bottle_status_16->setText(QString::number(bottle_16,10));
    ui->bottle_status_17->setText(QString::number(bottle_17,10));
    ui->bottle_status_18->setText(QString::number(bottle_18,10));
    ui->bottle_status_19->setText(QString::number(bottle_19,10));
    ui->bottle_status_20->setText(QString::number(bottle_20,10));
    ui->bottle_status_21->setText(QString::number(bottle_21,10));
    ui->bottle_status_22->setText(QString::number(bottle_22,10));
    ui->bottle_status_23->setText(QString::number(bottle_23,10));
    ui->bottle_status_24->setText(QString::number(bottle_24,10));



}
bool frmSampler::CleanSamplerStatus()
{
    QByteArray sendbuf;
    QByteArray readbuf;
    bool return_value=false;
    if(0==sampler.SampleMode)                  //SBC-6000协议
    {
        sendbuf[0]=0xAA;
        sendbuf[1]=0x36;
        sendbuf[2]=0xBB;
        myCom[COM_PORT-2]->readAll();
        myCom[COM_PORT-2]->write(sendbuf);
        return_value=true;
        //        myHelper::Sleep(1000);
//        readbuf=myCom[0]->readAll();
//        if(0xCC==readbuf[0]&&0xDD==readbuf[1]&&0xAA==readbuf[2]&&0xF0==readbuf[3]&&0xBB==readbuf[4]){
//            return true;
//        }else {
//            return false;
//        }
    }
    else if(1==sampler.SampleMode)     //北环科协议
    {
        sendbuf[0]=0x25;
        sendbuf[1]=0x43;
        sendbuf[2]=0x38;
        sendbuf[3]=0x0D;
        sendbuf[4]=0xAD;
        myCom[COM_PORT-2]->readAll();
        myCom[COM_PORT-2]->write(sendbuf);
        myHelper::Sleep(1000);
        readbuf=myCom[COM_PORT-2]->readAll();
        if(0x25==readbuf[0]&&0x43==readbuf[1]&&0x38==readbuf[2]&&0x79==readbuf[3]&&0x0D==readbuf[4]){
            return_value=true;
        }else {
            return_value=false;
        }
    }

    QString fileName=myApp::AppPath+"config_web.txt";
    QSettings *set=new QSettings(fileName,QSettings::IniFormat);
    set->beginGroup("WebConfig");
    myApp::CleanSampleCmd=0;
    set->setValue("CleanSampleCmd",myApp::CleanSampleCmd);
    set->endGroup();
    return return_value;

}

void frmSampler::on_btn_StartSampling_clicked()   //采样启动
{

        if(frmSampler::SampleFixFlow()==true){
            myHelper::ShowMessageBoxInfo("启动采样成功");
        }else {
            myHelper::ShowMessageBoxInfo("启动采样失败");
        }

}


void frmSampler::on_btn_Cancel_clicked()
{
    this->close();
}

void frmSampler::on_btn_GetSamplerStatus_clicked()
{
    if(GetSamplerStatus()) showbottle();
}

void frmSampler::on_btn_setsave_clicked()
{
    sampler.SampleMode=ui->comboBox_sampleMode->currentIndex();
    sampler.SampleFlow=ui->txtSampleFlow->text().toInt();
    sampler.SampleStartBottle=ui->comboBox_SampleStartBottle->currentText().toInt();
    myApp::WriteConfig();

}




void frmSampler::on_btn_CleanSamplerStatus_clicked()
{
    if(CleanSamplerStatus()==true){
        myHelper::ShowMessageBoxInfo("清空采样瓶状态成功");
    }else {
        myHelper::ShowMessageBoxInfo("清空采样瓶状态失败");
    }

}
