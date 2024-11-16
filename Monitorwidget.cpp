#include "MonitorWidget.h"
// #include "vector"

cv::Mat grabImg3;
bool Monitorm_bOpenDevice;
QImage* QmyImage ;
MV_CC_DEVICE_INFO_LIST Monitorm_stDevList;
CMvCamera *Monitorm_pcMyCamera = NULL;
MyThread *MonitormyThread = NULL;
Mat *MonitormyImage = NULL;

void MonitorWidget()
{
    delete MonitormyImage;
    if(Monitorm_pcMyCamera)
    {
        Monitorm_pcMyCamera->Close();
        delete Monitorm_pcMyCamera;
        Monitorm_pcMyCamera = NULL;
    }
    if(MonitormyThread->isRunning())
    {
        MonitormyThread->requestInterruption();
        MonitormyThread->wait();
        delete MonitormyThread;
    }
}

void MonitorinitWidget()
{
    //初始化变量
    Monitorm_bOpenDevice = false;

    //生成保存图片的文件夹
    QString imagePath = QDir::currentPath() + "/myImage/";
    QDir dstDir(imagePath);
    if(!dstDir.exists())
    {
        if(!dstDir.mkdir(imagePath))
        {
            qDebug()<<"创建Image文件夹失败！";
            //return;
        }
    }

    //图像指针对象
    MonitormyImage = new Mat();


    //线程对象实例化
    MonitormyThread = new MyThread();
    // connect(myThread,SIGNAL(signal_message()),this,SLOT(slot_display()));
    // connect(myThread,SIGNAL(signal_messImage(QImage)),this,SLOT(slot_displayImage(QImage)));
}

//保存图片
void MonitorsaveImage(QString format)
{
    //QPixmap方法保存在程序运行目录
    //format: .bmp .tif .png .jpg
    QString savePath = QDir::currentPath() + "/myImage/";
    QString curDate = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss.zzz");
    QString saveName = savePath + curDate + "." + format;
    qDebug()<<"saveName:"<<saveName;
}

//用于显示单张图像
void Monitordisplay(const Mat* imagePtr)
{
    qDebug()<<"single display ok";
    //判断是黑白、彩色图像
    // QImage* QmyImage = new QImage();
    // QmyImage = NULL;

    if(imagePtr->channels()>1)
    {

        *QmyImage = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_RGB888);

    }
    else
    {

        *QmyImage = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_Indexed8);
    }

    // *QmyImage = (*QmyImage).scaled(ui->lb_show->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    //QImage保存图像
    //QmyImage->save(saveName);

    //显示图像
    // ui->lb_show->setPixmap(QPixmap::fromImage(*QmyImage));
    // delete QmyImage;
    return;
}

void slot_display()
{
    qDebug()<<"continuous display ok";
    //判断是黑白、彩色图像
    QImage *QmyImage = new QImage();
    if(MonitormyImage->channels()>1)
    {
        *QmyImage = QImage((const unsigned char*)(MonitormyImage->data),MonitormyImage->cols,MonitormyImage->rows,QImage::Format_RGB888);
    }
    else
    {
        *QmyImage = QImage((const unsigned char*)(MonitormyImage->data),MonitormyImage->cols,MonitormyImage->rows,QImage::Format_Indexed8);
    }

    // *QmyImage = (*QmyImage).scaled(ui->lb_show->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    //显示图像
    // ui->lb_show->setPixmap(QPixmap::fromImage(*QmyImage));
    delete QmyImage;
}

void slot_displayImage(QImage MonitormyImage)
{
    //显示图像
    // myImage = (myImage).scaled(ui->lb_show->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    // ui->lb_show->setPixmap(QPixmap::fromImage(myImage));
}

void on_pb_find_clicked()
{

    // ui->cb_devices->clear();
    memset(&Monitorm_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    //枚举子网内所有设备
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE,&Monitorm_stDevList);
    if (MV_OK != nRet)
    {
        return;
    }
    for(unsigned int i = 0; i < Monitorm_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = Monitorm_stDevList.pDeviceInfo[i];
        QString strModelName = "";

        if(pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            strModelName = (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName;
        }
        else if(pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            strModelName = (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName;
        }
        else
        {
            // QMessageBox::warning(this,"警告","未知设备枚举！");
            return;
        }
        // qDebug()<<"strModelName:"<<strModelName;
        // ui->cb_devices->addItem(strModelName);
    }

}

void on_pb_open_clicked()
{
    if(Monitorm_bOpenDevice)
    {
        return;
    }
    // QString deviceModel = ui->cb_devices->currentText();
    // if(deviceModel == "")
    {
        // QMessageBox::warning(this,"警告","请选择设备！");
        // return;
    }
    Monitorm_pcMyCamera = new CMvCamera;
    if(NULL == Monitorm_pcMyCamera)
    {
        return;
    }
    // int nIndex = ui->cb_devices->currentIndex();
    int nIndex = 0;

    //打开设备
    // int nRet = Monitorm_pcMyCamera->Open(Monitorm_stDevList.pDeviceInfo[nIndex]);
    int nRet = Monitorm_pcMyCamera->Open(Monitorm_stDevList.pDeviceInfo[nIndex]);
    qDebug()<<"Connect:"<<nRet;
    if(MV_OK != nRet)
    {
        delete Monitorm_pcMyCamera;
        Monitorm_pcMyCamera = NULL;
        // QMessageBox::warning(this,"警告","打开设备失败！");
        printf("警告打开设备失败！/n");

        return;
    }

    //设置为触发模式
    Monitorm_pcMyCamera->SetEnumValue("TriggerMode",1);
    //设置触发源为软触发
    Monitorm_pcMyCamera->SetEnumValue("TriggerSource",7);
    //设置曝光时间
    Monitorm_pcMyCamera->SetFloatValue("ExposureTime",300000);
    //开启相机采集
    Monitorm_pcMyCamera->StartGrabbing();

    MonitormyThread->getCameraPtr(Monitorm_pcMyCamera);
    MonitormyThread->getImagePtr(MonitormyImage);

    Monitorm_bOpenDevice = true;

}

void on_pb_close_clicked()
{
    if(Monitorm_pcMyCamera)
    {
        Monitorm_pcMyCamera->Close();
        delete Monitorm_pcMyCamera;
        Monitorm_pcMyCamera = NULL;
        Monitorm_bOpenDevice = false;
    }
}

void on_pb_single_clicked()
{
    if(!Monitorm_bOpenDevice)
    {
        // QMessageBox::warning(this,"警告","采集失败,请打开设备！");
        return;
    }

    // Mat *image = new Mat();
    //发送软触发
    Monitorm_pcMyCamera->CommandExecute("TriggerSoftware");
    //读取Mat格式的图像
    // Monitorm_pcMyCamera->ReadBuffer(*image);
    Monitorm_pcMyCamera->ReadBuffer(grabImg3);
    // Monitordisplay(image);

    //释放指针
    // delete image;

    return;
}

void on_pb_start_clicked()
{
    if(!Monitorm_bOpenDevice)
    {
        // QMessageBox::warning(this,"警告","采集失败,请打开设备！");
        return;
    }

    if(!MonitormyThread->isRunning())
    {
        MonitormyThread->start();
    }
    return;
}

void on_pb_stop_clicked()
{
    if(MonitormyThread->isRunning())
    {
        MonitormyThread->requestInterruption();
        MonitormyThread->wait();
    }
}

void on_pb_saveBmp_clicked()
{
    MonitorsaveImage("bmp");   //文件较大
}

void on_pb_savePng_clicked()
{
    MonitorsaveImage("png");   //文件较小
}

