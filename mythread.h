#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include "cmvcamera.h"

class MyThread : public QThread
{
    Q_OBJECT

public:
    explicit MyThread(QObject *parent = nullptr);
     ~MyThread();

    void run();
    void getCameraPtr(CMvCamera *camera);
    void getImagePtr(cv::Mat *image);

signals:
    void signal_message();
    void signal_messImage(QImage myImage);

private:
    CMvCamera *cameraPtr = NULL;
    cv::Mat *imagePtr = NULL;
    QImage *myImage = NULL;

};

#endif // MYTHREAD_H
