#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QMessageBox>
#include <QTextCodec>
#include <QFileDialog>

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

#include "RobotController.h"
#include "TrackController.h"
#include "IOController.h"
#include "GlobalVariable.h"
#include "WorkProcess.h"
#include "Container.h"
#include "tinyxml_2_6_2/tinyxml/tinyxml.h"

#pragma comment(lib, "ws2_32.lib")


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum LightState {
    LightGray,
    LightGreen,
    LightRed
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

    void loading(int shelvesId, int boxId, int fixtureId, const std::string& deviceID = "", const std::string& cellID = "", const std::string& channelID = "", const std::string& stepFilename = "");
    void unloading(int shelvesId, int boxId, int fixtureId);

    void initialization();
    int startDetection(std::string devid, std::string subdevid, std::string chlid, std::string filename);
    int getDetectionStatus();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<RobotController> m_robotController;
    std::unique_ptr<TrackController> m_trackController;
    std::unique_ptr<IOController> m_ioController;
    QTimer* m_comTimer;
    QTimer* m_clockTimer;
    QTimer* m_detectTimer;

    bool m_stopProcess;
};
#endif // MAINWINDOW_H
