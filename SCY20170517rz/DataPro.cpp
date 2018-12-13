#include <DataPro.h>

#include <QFile>
#include <QLabel>
#include <QEvent>
#include <QTimer>
#include <QPixmap>
#include <QSettings>
#include <QTextStream>

DataPro::DataPro(QObject *parent)
    : QObject(parent)
{
    init();
}

DataPro::~DataPro()
{

}

void DataPro::init()
{
    myTimerId=0;

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_timeout()));
    timer->start(10000);


}

void DataPro::MinDataEvent(QTimer)
{

}

bool DataPro::eventFilter(QObject *obj, QEvent *event)
{
    //判断事件类型
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseMove
        || event->type() == QEvent::MouseButtonPress) {
        brightness=0;
        DataPro::WriteBacklight(brightness);

    }

    return QObject::eventFilter(obj, event);
}
void DataPro::slot_timeout()
{
    brightness++;
    if(brightness>8){
        brightness=8;
        timer->stop();
    }
    DataPro::WriteBacklight(brightness);
    timer->start(5000);
}
//brightness=0:最亮  brightness=8最暗
void DataPro::WriteBacklight(int brightness)
{
     QFile writeFile("/sys/class/backlight/backlight/brightness");
     if(writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
     {
        QTextStream stream(&writeFile);
        stream<<QString("%1").arg(brightness) <<"\n";
     }
 writeFile.close();
}

