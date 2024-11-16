#include "mythread.h"

MyThread::MyThread(QObject *parent)
    : QThread{parent}
{
    myImage = new QImage();
}

MyThread::~MyThread()
{
    delete myImage;
    if(cameraPtr == NULL)
    {
        delete cameraPtr;
    }
    if(imagePtr == NULL)
    {
        delete imagePtr;
    }
}

void MyThread::getCameraPtr(CMvCamera *camera)
{
    cameraPtr = camera;
}

void MyThread::getImagePtr(cv::Mat *image)
{
    imagePtr = image;
}

void MyThread::run()
{
    if(cameraPtr == NULL)
    {
        return;
    }
    if(imagePtr == NULL)
    {
        return;
    }

    while(!isInterruptionRequested())
    {
        qDebug()<<"SoftTrigger:"<<cameraPtr->CommandExecute("TriggerSoftware");
        qDebug()<<"ReadBuffer:"<<cameraPtr->ReadBuffer(*imagePtr);

        //先发送好再处理
        //emit signal_message();
        //msleep(10);

        //先处理好再发送
        if(imagePtr->channels()>1)
        {
            *myImage = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_RGB888);
        }
        else
        {
            *myImage = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_Indexed8);
        }
        emit signal_messImage(*myImage);
        msleep(10);

        //耗时操作，需要放到线程中保存
        //QImage保存图像
        //QString savePath = QDir::currentPath() + "/myImage/";
        //QString curDate = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss.zzz");
        //QString saveName = savePath + curDate + ".png";
        //qDebug()<<"saveName:"<<saveName;
        //myImage->save(saveName);
    }
}
