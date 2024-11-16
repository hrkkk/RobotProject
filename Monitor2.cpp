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
PROCGETCONSOLEWINDOW GetConsoleWindowAPI2;
// class monitor2
// {
// public:
//     static LONG lUserID;
// };
static LONG lUserID2;
static LONG lRealPlayHandle2;
//extern LONG lUserID;
cv::Mat grabImg2;
HWND hWnd2;
//extern LONG lUserID;
//LONG lPort; //全局的播放库port号


void CALLBACK g_ExceptionCallBack2(DWORD dwType, LONG lUserID2, LONG lHandle, void *pUser)
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

void open_monitor2(int argc2, char *argv2[])
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
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack2, NULL);

    //---------------------------------------
    // 获取控制台窗口句柄
    HMODULE hKernel32 = GetModuleHandle("kernel32");
    GetConsoleWindowAPI2 = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");

    //---------------------------------------
    // 注册设备
    // extern LONG lUserID;

    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.168.1.65"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "PanRou2024"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID2 = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID2 < 0)
    {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return;
    }
    return;

}

void watch_monitor2(int argc2, char *argv2[])
{
    //---------------------------------------
    //启动预览并设置回调数据流
    // LONG lRealPlayHandle;
    //HWND hWnd = GetConsoleWindowAPI2();     //获取窗口句柄
    NET_DVR_PREVIEWINFO struPlayInfo2 = {0};
    struPlayInfo2.hPlayWnd = hWnd2;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo2.lChannel     = 1;       //预览通道号
    struPlayInfo2.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo2.dwLinkMode   = 0;       //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo2.bBlocked     = 1;       //0- 非阻塞取流，1- 阻塞取流

    lRealPlayHandle2 = NET_DVR_RealPlay_V40(lUserID2, &struPlayInfo2, NULL, NULL);
    if (lRealPlayHandle2== -1){
        printf("lRealPlayHandle2 empty\n");
    }
    std::vector<char> inputData2(640*368*3); //创建一个数组，大小为图像的H*W*4 + 54，用来保存图像数据
    DWORD size2= 0; //返回的实际图像大小
    DWORD* psize2 = &size2;  //指向实际图像大小的指针

    auto ret2 = NET_DVR_CapturePictureBlock_New(lRealPlayHandle2, inputData2.data(), (DWORD)inputData2.size(), psize2); //通过这个函数获取一帧图像数据
    // std::cout << ret2 << std::endl;
    // printf("%d\n", size2);
    cv::_InputArray pic_arr(inputData2); //将图像数据转换为OpenCV的数组
    grabImg2 = cv::imdecode(pic_arr, cv::IMREAD_COLOR); //OpenCV数组转换为Mat对象

    // if (grabImg2.empty()) {
    //     printf("empty\n");
    // }


    if (lRealPlayHandle2 < 0)
    {
        printf("NET_DVR_RealPlay_V40 error\n");
        NET_DVR_Logout(lUserID2);
        NET_DVR_Cleanup();
        return;
    }
}

void close_monitor2(int argc2, char *argv2[])
{
    Sleep(10);
    //---------------------------------------
    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle2);
    //注销用户
    NET_DVR_Logout(lUserID2);
    //释放SDK资源
    NET_DVR_Cleanup();

    // w.show();
    // return a.exec();

}
