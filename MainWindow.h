#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// QT库文件
#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QMessageBox>
#include <QTextCodec>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QSettings>
// C++库文件
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>
#include <winsock.h>
#include <windows.h>
#include <iostream>
// 自定义头文件
#include "TaskListModel.h"
#include "RobotController.h"
#include "TrackController.h"
#include "IOController.h"
#include "GlobalVariable.h"
#include "tinyxml_2_6_2/tinyxml/tinyxml.h"
#include "Monitor1.h"
#include "Monitor2.h"
#include "MonitorWidget.h"
#include "cmvcamera.h"

#pragma comment(lib, "ws2_32.lib")


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class LightState {
    LightGray,
    LightGreen,
    LightRed
};

enum class ProcessState {
    RUNING,
    PAUSED,
    STOPPED
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setLightState(QWidget* parent, const QString& lightName, LightState state);
    void setClickedLightState(QWidget* parent, const QString& lightName, LightState state);

    void initTrackUI();
    void initRobotUI();
    void initModuleUI();
    void switchPage(int index);

    // 加载通道号文件
    void loadChannelFile(int fixtureID);

    void loading(int shelvesId, int boxId, int fixtureId);
    void unloading(int shelvesId, int boxId, int fixtureId);

    void initialization();
    int startDetection(std::string devid, std::string subdevid, std::string chlid, std::string filename);
    std::vector<std::string> getDetectionStatus(const std::vector<std::vector<int>>& channelsList);

protected:
    void closeEvent(QCloseEvent* event) override;



private:
    Ui::MainWindow *ui;
    std::unique_ptr<RobotController> m_robotController;
    std::unique_ptr<TrackController> m_trackController;
    std::unique_ptr<IOController> m_ioController;
    QTimer* m_comTimer;
    QTimer* m_clockTimer;
    QTimer* m_detectTimer;
    QTimer* m_ioComTimer;
    QTimer* m_taskTimer;
    QTimer* m_stopTimer;
    TaskListModel* m_taskList;
    ProcessState m_processState;
    QDateTime m_startingTime;
};
#endif // MAINWINDOW_H
