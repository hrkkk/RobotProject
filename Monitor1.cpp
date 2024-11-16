//#include <QApplication>
#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include "vector"
// #include "widget.h"
#include "Monitor1.h"
#include <time.h>

using namespace std;

typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindowAPI1;
// class monitor1
// {
// public:
//     static LONG lUserID;
// };
static LONG lUserID1;
static LONG lRealPlayHandle1;
//extern LONG lUserID;
cv::Mat grabImg1;
HWND hWnd1;
//LONG lPort; //全局的播放库port号

// 报警信号
int g_warningSignal = 0;
int g_enterSignal = 0;
int g_leaveSignal = 0;

// 时间解析宏定义
#define GET_YEAR(_time_)      (((_time_)>>26) + 2000)
#define GET_MONTH(_time_)     (((_time_)>>22) & 15)
#define GET_DAY(_time_)       (((_time_)>>17) & 31)
#define GET_HOUR(_time_)      (((_time_)>>12) & 31)
#define GET_MINUTE(_time_)    (((_time_)>>6)  & 63)
#define GET_SECOND(_time_)    (((_time_)>>0)  & 63)

// void CALLBACK MessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
// {
//     switch (lCommand)
//     {
//     case COMM_ISAPI_ALARM: //协议透传报警信息，分离模式
//     {
//         NET_DVR_ALARM_ISAPI_INFO struISAPIAlarm = { 0 };
//         memcpy(&struISAPIAlarm, pAlarmInfo, sizeof(NET_DVR_ALARM_ISAPI_INFO));

//         //解析JSON数据
//         if (struISAPIAlarm.dwAlarmDataLen > 0)
//         {
//             printf("COMM_ISAPI_ALARM报警事件信息数据: \n%s\n", UTF8ToAnsi(struISAPIAlarm.pAlarmData));
//         }

//         //保存二进制图片数据
//         char cFilename[256] = { 0 };

//         SYSTEMTIME t;
//         GetLocalTime(&t);
//         char chTime[128];
//         sprintf(chTime, "%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

//         if (struISAPIAlarm.pPicPackData != NULL)
//         {
//             int i = 0;
//             while (i <= struISAPIAlarm.byPicturesNumber - 1)
//             {
//                 DWORD dwWrittenBytes = 0;
//                 if (1 == ((NET_DVR_ALARM_ISAPI_PICDATA *)struISAPIAlarm.pPicPackData)[i].byPicType)
//                 {
//                     sprintf(cFilename, "PicData[%s]_%s_%d.jpg", chTime, ((NET_DVR_ALARM_ISAPI_PICDATA *)struISAPIAlarm.pPicPackData)[i].szFilename, rand() % GetTickCount());
//                 }
//                 else
//                 {
//                     sprintf(cFilename, "PicData[%s]_%s_%d.dat", chTime, ((NET_DVR_ALARM_ISAPI_PICDATA *)struISAPIAlarm.pPicPackData)[i].szFilename, rand() % GetTickCount());
//                 }

//                 FILE *fAlarmPic = fopen(cFilename, "wb");
//                 fwrite(((NET_DVR_ALARM_ISAPI_PICDATA *)struISAPIAlarm.pPicPackData)[i].pPicData, ((NET_DVR_ALARM_ISAPI_PICDATA *)struISAPIAlarm.pPicPackData)[i].dwPicLen, 1, fAlarmPic);
//                 fclose(fAlarmPic);
//                 fAlarmPic = NULL;
//             }
//         }
//     }
//     break;
//     case COMM_VCA_ALARM: //智能检测通用报警，非分离模式
//     {
//         if (pAlarmInfo == NULL)
//         {
//             return;
//         }
//         //如果带图片二进制数据，需要自己解析表单格式的报文，不能直接转成字符串
//         printf("COMM_VCA_ALARM智能检测通用报警, 报警数据内容：\n%s\n", UTF8ToAnsi(pAlarmInfo));
//     }
//     break;
//     default:
//         printf("其他报警，报警信息类型: 0x%x\n", lCommand);
//         break;
//     }
//     return;
// }


void CALLBACK cbMessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
    switch (lCommand) {
        case COMM_ALARM_RULE: //异常行为识别报警信息
        {
            NET_VCA_RULE_ALARM struVcaAlarm = { 0 };
            memcpy(&struVcaAlarm, pAlarmInfo, sizeof(NET_VCA_RULE_ALARM));

            NET_DVR_TIME struAbsTime = { 0 };
            struAbsTime.dwYear = GET_YEAR(struVcaAlarm.dwAbsTime);
            struAbsTime.dwMonth = GET_MONTH(struVcaAlarm.dwAbsTime);
            struAbsTime.dwDay = GET_DAY(struVcaAlarm.dwAbsTime);
            struAbsTime.dwHour = GET_HOUR(struVcaAlarm.dwAbsTime);
            struAbsTime.dwMinute = GET_MINUTE(struVcaAlarm.dwAbsTime);
            struAbsTime.dwSecond = GET_SECOND(struVcaAlarm.dwAbsTime);

            //保存抓拍场景图片
            if (struVcaAlarm.dwPicDataLen > 0 && struVcaAlarm.pImage != NULL)
            {
                char cFilename[256] = { 0 };
                HANDLE hFile;
                DWORD dwReturn;

                char chTime[128];
                sprintf(chTime, "%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour, struAbsTime.dwMinute, struAbsTime.dwSecond);

                sprintf(cFilename, "VcaAlarmPic[%s][%s].jpg", struVcaAlarm.struDevInfo.struDevIP.sIpV4, chTime);

                hFile = CreateFileW((LPCWSTR)cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    break;
                }
                WriteFile(hFile, struVcaAlarm.pImage, struVcaAlarm.dwPicDataLen, &dwReturn, NULL);
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
            }

            WORD wEventType = struVcaAlarm.struRuleInfo.wEventTypeEx;

            // printf("\n\n");
            // printf("异常行为识别报警[0x%x]: Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] Dev[ip:%s,port:%d,ivmsChan:%d] Smart[%d] EventTypeEx[%d]\n", \
            //        lCommand, struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour, struAbsTime.dwMinute, \
            //                              struAbsTime.dwSecond, struVcaAlarm.struDevInfo.struDevIP.sIpV4, struVcaAlarm.struDevInfo.wPort, \
            //                                        struVcaAlarm.struDevInfo.byIvmsChannel, struVcaAlarm.bySmart, wEventType);

            NET_VCA_TARGET_INFO tmpTargetInfo;
            memcpy(&tmpTargetInfo, &struVcaAlarm.struTargetInfo, sizeof(NET_VCA_TARGET_INFO));
            // printf("目标信息:ID[%d]RECT[%f][%f][%f][%f]\n",
            //        tmpTargetInfo.dwID, tmpTargetInfo.struRect.fX, tmpTargetInfo.struRect.fY,
            //        tmpTargetInfo.struRect.fWidth, tmpTargetInfo.struRect.fHeight);

            cout <<"outputWARNING" << endl;
            if (g_warningSignal == 0) {
                g_warningSignal = 1;
            }

            // switch (wEventType) {
            //     case ENUM_VCA_EVENT_INTRUSION: { //区域入侵报警
            //         printf("区域入侵报警: wDuration[%d], Sensitivity[%d]\n", \
            //         struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.wDuration, \
            //         struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.bySensitivity);
            //         printf("规则区域: \n");
            //         NET_VCA_POLYGON tempPolygon;
            //         memcpy(&tempPolygon, &struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.struRegion, sizeof(NET_VCA_POLYGON));
            //         for (int i = 0; i < (int)tempPolygon.dwPointNum; i++)
            //         {
            //             printf("[%f, %f] ", tempPolygon.struPos[i].fX, tempPolygon.struPos[i].fY);
            //         }
            //         break;
            //     }
            //     default: {
            //         printf("其他报警，报警信息类型: 0x%x\n", lCommand);
            //         break;
            //     }
            // }
            break;
        }
        default: {
            printf("其他报警，报警信息类型: 0x%x\n", lCommand);
            break;
        }
    }

    return;
}





void CALLBACK g_ExceptionCallBack1(DWORD dwType, LONG lUserID1, LONG lHandle, void *pUser)
{
    char tempbuf[256] = {0};
    switch(dwType)
    {
    case EXCEPTION_RECONNECT:    //预览时重连
        printf("----------reconnect--------%d\n", time(NULL));
        break;
    default:
        break;
    }
}

void output1()
{
    // std::vector<char> inputData(3840*2160*4 + 54); //创建一个数组，大小为图像的H*W*4 + 54，用来保存图像数据
    // DWORD size= 0; //返回的实际图像大小
    // DWORD* psize = &size;  //指向实际图像大小的指针
    // NET_DVR_CapturePictureBlock_New(lRealPlayHandle1, inputData.data(), (DWORD)inputData.size(), psize); //通过这个函数获取一帧图像数据

    // cv::_InputArray pic_arr(inputData); //将图像数据转换为OpenCV的数组
    // grabImg = cv::imdecode(pic_arr, cv::IMREAD_COLOR); //OpenCV数组转换为Mat对象

    // printf("1122\n");
    // if (grabImg.empty()) {
    //     printf("empty\n");
    // }

    // //通过以下代码，把opencv的Mat转换为QImage，然后在Qt的QLabel控件上进行显示
    // QImage qgrabImg = QImage((const unsigned char*)(grabImg.data), grabImg.cols, grabImg.rows, grabImg.cols*grabImg.channels(), QImage::Format_RGB888);   // Mat为BGR颜色模式;
    // qgrabImg = qgrabImg.rgbSwapped();
    // ui->grabImg->setPixmap(QPixmap::fromImage(qgrabImg));

}

MSGCallBack DataCallback;
void *pUserData;

void open_monitor1(int argc1, char *argv1[])
{

    // Widget w;
    //---------------------------------------
    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //---------------------------------------
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack1, NULL);

    //---------------------------------------
    // 获取控制台窗口句柄
    HMODULE hKernel32 = GetModuleHandle("kernel32");
    GetConsoleWindowAPI1 = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");

    //---------------------------------------
    // 注册设备
    // extern LONG lUserID;

    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.168.1.64"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "PanRou2024"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID1 = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID1 < 0)
    {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        cout <<"Login failed, error code: %d\n" << endl;
        NET_DVR_Cleanup();
        return;
    }

    //设置报警回调函数
    /*注：多台设备对接时也只需要调用一次设置一个回调函数，不支持不同设备的事件在不同的回调函数里面返回*/
    NET_DVR_SetDVRMessageCallBack_V50(0, cbMessageCallback, NULL);

    //启用布防
    LONG lHandle;
    NET_DVR_SETUPALARM_PARAM struAlarmParam = { 0 };
    struAlarmParam.dwSize = sizeof(struAlarmParam);
    //其他报警布防参数不需要设置，不支持
    // lHandle= NET_DVR_SetupAlarmChan_V30(lUserID1);

    // //启动监听设置报警回调函数
    // int lHandle1 = -1;
    // lHandle1 = NET_DVR_StartListen_V30("10.17.36.38", 8000, MessageCallback, NULL);

    lHandle = NET_DVR_SetupAlarmChan_V41(lUserID1, &struAlarmParam);
    if (lHandle < 0)
    {
        // printf("NET_DVR_SetupAlarmChan_V41 error, %d\n", NET_DVR_GetLastError());
        cout <<"NET_DVR_SetupAlarmChan_V41 error, %d\n" << endl;
        NET_DVR_Logout(lUserID1);
        NET_DVR_Cleanup();
        return;
    }
    else
    {
        // cout <<"Monitor ok" << endl;
    }

    return;

}
void watch_monitor1(int argc1, char *argv1[])
{
    //---------------------------------------
    //启动预览并设置回调数据流
    // LONG lRealPlayHandle;
    // HWND hWnd = GetConsoleWindowAPI1();     //获取窗口句柄
    NET_DVR_PREVIEWINFO struPlayInfo1 = {0};
    struPlayInfo1.hPlayWnd = hWnd1;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo1.lChannel     = 1;       //预览通道号
    struPlayInfo1.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo1.dwLinkMode   = 0;       //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo1.bBlocked     = 1;       //0- 非阻塞取流，1- 阻塞取流
    // printf("out1\n");
    lRealPlayHandle1 = NET_DVR_RealPlay_V40(lUserID1, &struPlayInfo1, NULL, NULL);
    // if (lRealPlayHandle1== -1){
    //     printf("empty\n");
    // }
    std::vector<char> inputData1(640*368*3); //创建一个数组，大小为图像的H*W*4 + 54，用来保存图像数据
    DWORD size1= 0; //返回的实际图像大小
    DWORD* psize1 = &size1;  //指向实际图像大小的指针

    auto ret1 = NET_DVR_CapturePictureBlock_New(lRealPlayHandle1, inputData1.data(), (DWORD)inputData1.size(), psize1); //通过这个函数获取一帧图像数据
    // std::cout << ret1 << std::endl;
    // printf("%d\n", size);
    cv::_InputArray pic_arr(inputData1); //将图像数据转换为OpenCV的数组
    grabImg1 = cv::imdecode(pic_arr, cv::IMREAD_COLOR); //OpenCV数组转换为Mat对象



    if (lRealPlayHandle1 < 0)
    {
        printf("NET_DVR_RealPlay_V40 error\n");
        NET_DVR_Logout(lUserID1);
        NET_DVR_Cleanup();
        return;
    }



}

void close_monitor1(int argc1, char *argv1[])
{
    Sleep(10);
    //---------------------------------------
    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle1);
    //注销用户
    NET_DVR_Logout(lUserID1);
    //释放SDK资源
    NET_DVR_Cleanup();

    // w.show();
    // return a.exec();

}
