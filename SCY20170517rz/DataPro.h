#ifndef DATAPRO_H
#define DATAPRO_H
#ifndef QOBJECT_H
#include <QObject>
#endif

class QTimer;
class DataPro : public QObject
{
    Q_OBJECT

public:
    DataPro(QObject *parent = NULL);
    ~DataPro();

//protected slots:
//    void slot_timeout();

protected:
    //初始化屏保参数
    void init();


    //事件接收处理函数，由installEventFilter调用方在接收到事件时调用
    bool eventFilter(QObject *watched, QEvent *event);

private:
    //定时器
    QTimer   *timer;
    int myTimerId;
};




#endif // DATAPRO_H
