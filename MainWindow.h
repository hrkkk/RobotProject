#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QMessageBox>
#include <QTextCodec>

#include <memory>

#include "RobotController.h"
#include "TrackController.h"
#include "IOController.h"
#include "GlobalVariable.h"
#include "WorkProcess.h"
#include "Container.h"


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

    void loading(int shelvesId, int boxId, int fixtureId);
    void unloading(int shelvesId, int boxId, int fixtureId);

private:
    Ui::MainWindow *ui;
    std::unique_ptr<RobotController> m_robotController;
    std::unique_ptr<TrackController> m_trackController;
    std::unique_ptr<IOController> m_ioController;
    QTimer* m_timer;
    QTimer* m_clockTimer;
    QString m_dragSourceWidget;
    QString m_dragTargetWidget;
    QMap<QString, QLabel*> m_pallets;
    bool m_stopProcess;
};
#endif // MAINWINDOW_H
