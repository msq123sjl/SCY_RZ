#ifndef MYSCREENSAVER_H
#define MYSCREENSAVER_H

#ifndef QOBJECT_H
#include <QObject>
#endif

class QTimer;
class MyScreenSaver : public QObject
{
    Q_OBJECT

public:
    MyScreenSaver(QObject *parent = NULL);
    ~MyScreenSaver();

protected slots:
    void slot_timeout();

protected:
    //初始化屏保参数
    void init();
    void WriteBacklight(int brightness);

    //事件接收处理函数，由installEventFilter调用方在接收到事件时调用
    bool eventFilter(QObject *watched, QEvent *event);

private:
    //定时器
    QTimer   *timer;

};

#endif // MYSCREENSAVER_H
