#include <myscreensaver.h>

#include <QFile>
#include <QLabel>
#include <QEvent>
#include <QTimer>
#include <QPixmap>
#include <QSettings>
#include <QTextStream>

MyScreenSaver::MyScreenSaver(QObject *parent)
    : QObject(parent)
{
    init();
}

MyScreenSaver::~MyScreenSaver()
{
}
int brightness=0;

void MyScreenSaver::init()
{
    //设置初始文件值
    MyScreenSaver::WriteBacklight(brightness);
    //设置并启动timer。如果超过30s，则一直显示屏保并不再触发直到再次刷新定时器
    timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_timeout()));
    timer->start(10000);
}
bool MyScreenSaver::eventFilter(QObject *obj, QEvent *event)
{
    //判断事件类型
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseMove
        || event->type() == QEvent::MouseButtonPress) {
        brightness=0;
        MyScreenSaver::WriteBacklight(brightness);
        //有鼠标或键盘事件则重置timer
        timer->stop();

        timer->start(30000);

    }

    return QObject::eventFilter(obj, event);
}
void MyScreenSaver::slot_timeout()
{
    brightness++;
    if(brightness>8){
        brightness=8;
        timer->stop();
    }
    MyScreenSaver::WriteBacklight(brightness);
    timer->start(5000);
}
//brightness=0:最亮  brightness=8最暗
void MyScreenSaver::WriteBacklight(int brightness)
{
     QFile writeFile("/sys/class/backlight/backlight/brightness");
     if(writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
     {
        QTextStream stream(&writeFile);
        stream<<QString("%1").arg(brightness) <<"\n";
     }
     writeFile.close();
}
