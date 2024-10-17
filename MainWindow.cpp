#include "MainWindow.h"
#include "./ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_robotController(std::make_unique<RobotController>()),
    m_trackController(std::make_unique<TrackController>()),
    m_ioController(std::make_unique<IOController>()),
    m_timer(new QTimer()),
    m_clockTimer(new QTimer()),
    m_stopProcess(true)
{
    ui->setupUi(this);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    this->showMaximized();
    ui->tabWidget->tabBar()->hide();
    switchPage(1);

    connect(ui->btn_page1, &QPushButton::clicked, this, [=]() {
        switchPage(0);
    });
    connect(ui->btn_page2, &QPushButton::clicked, this, [=]() {
        switchPage(1);
    });
    connect(ui->btn_page3, &QPushButton::clicked, this, [=]() {
        switchPage(2);
    });
    connect(ui->btn_page4, &QPushButton::clicked, this, [=]() {
        switchPage(3);
    });
    connect(ui->btn_page5, &QPushButton::clicked, this, [=]() {
        switchPage(4);
    });

    // 定时读取数据
    connect(m_timer, &QTimer::timeout, this, [=]() {
        // 每秒读取一次地轨PLC数据
        if (m_trackController && m_trackController->isConnected()) {
            static int last = 1;
            m_trackController->requestFeedback(last);
            last = !last;
        }
        // 每秒读取一次机械臂PLC数据
        if (m_robotController && m_robotController->isConnected()) {
            m_robotController->requestFeedback();
        }
        // 每秒读取一次IO模块数据
        if (m_ioController && m_ioController->isConnected()) {
            static int last = 1;
            m_ioController->requestFeedback(last);
            last = !last;
        }
    });

    initTrackUI();
    initRobotUI();
    initModuleUI();

    // 记录开机时间
    connect(m_clockTimer, &QTimer::timeout, this, [=]() {
        // 获取当前时间
        ui->label_currTime->setText("2024 - 07 - 13  15:14");
    });

    if (m_clockTimer) {
        m_clockTimer->setInterval(1000);
        m_clockTimer->start();
    }

    connect(ui->btn_load, &QPushButton::clicked, this, [=]() {
        int shelvesId = ui->cBox_shelvesId->currentIndex() + 1;      // 1 - 12
        int boxId = ui->cBox_boxId->currentIndex() + 1;              // 1 - 3
        int fixtureId = ui->cBox_fixtureId->currentIndex() + 1;      // 1 - 6
        m_stopProcess = false;
        loading(shelvesId, boxId, fixtureId);
    });
    connect(ui->btn_unload, &QPushButton::clicked, this, [=]() {
        int shelvesId = ui->cBox_shelvesId->currentIndex() + 1;      // 1 - 12
        int boxId = ui->cBox_boxId->currentIndex() + 1;              // 1 - 3
        int fixtureId = ui->cBox_fixtureId->currentIndex() + 1;      // 1 - 6
        m_stopProcess = false;
        unloading(shelvesId, boxId, fixtureId);
    });

    this->setAcceptDrops(true);

    QList<Container*> containers;
    for (int i = 1; i <= 12; ++i) {
        containers.append(this->findChild<Container*>("container_shelves" + QString::number(i)));
    }
    for (int i = 1; i <= 6; ++i) {
        containers.append(this->findChild<Container*>("container_fixture" + QString::number(i)));
    }
    for (auto entry : containers) {
        connect(entry, &Container::sig_dragStart, this, [=](const QString& sourceWidget) {
            m_dragSourceWidget = sourceWidget;
        });
        connect(entry, &Container::sig_dragFinish, this, [=](const QString& targetWidget) {
            m_dragTargetWidget = targetWidget;
            if (m_dragTargetWidget == "") {
                m_dragSourceWidget = "";
            } else {
                if (m_dragSourceWidget.contains("shelves") && m_dragTargetWidget.contains("fixture")) {
                    // 从货架搬运到温控箱
                    QRegExp regex("\\d+$");
                    regex.indexIn(m_dragSourceWidget);
                    int shelvesId = regex.cap(0).toInt();
                    regex.indexIn(m_dragTargetWidget);
                    int fixtureId = regex.cap(0).toInt();

                    QMessageBox msgBox;
                    msgBox.setText(QStringLiteral("是否要将托盘从%1号货架移至%2号治具?").arg(shelvesId).arg(fixtureId));
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    int ret = msgBox.exec();
                    if (ret == QMessageBox::Yes) {
                        Container* srcContainer = findChild<Container*>(m_dragSourceWidget);
                        Container* dstContainer = findChild<Container*>(m_dragTargetWidget);
                        QLabel* pallet = m_pallets[m_dragSourceWidget];
                        m_pallets.remove(m_dragSourceWidget);
                        m_pallets.insert(m_dragTargetWidget, pallet);
                        srcContainer->removeWidget(pallet);
                        dstContainer->addWidget(pallet);

                        ui->label_src->setText(QStringLiteral("货架#%1").arg(shelvesId));
                        ui->label_dst->setText(QStringLiteral("治具#%1").arg(fixtureId));

                        loading(shelvesId, 1, fixtureId);
                    }
                } else if (m_dragSourceWidget.contains("fixture") && m_dragTargetWidget.contains("shelves")) {
                    // 从温控箱搬运到货架
                    QRegExp regex("\\d+$");
                    regex.indexIn(m_dragSourceWidget);
                    int fixtureId = regex.cap(0).toInt();
                    regex.indexIn(m_dragTargetWidget);
                    int shelvesId = regex.cap(0).toInt();

                    QMessageBox msgBox;
                    msgBox.setText(QStringLiteral("是否要将托盘从%1号治具移至%2号货架?").arg(fixtureId).arg(shelvesId));
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    int ret = msgBox.exec();
                    if (ret == QMessageBox::Yes) {
                        Container* srcContainer = findChild<Container*>(m_dragSourceWidget);
                        Container* dstContainer = findChild<Container*>(m_dragTargetWidget);
                        QLabel* pallet = m_pallets[m_dragSourceWidget];
                        m_pallets.remove(m_dragSourceWidget);
                        m_pallets.insert(m_dragTargetWidget, pallet);
                        srcContainer->removeWidget(pallet);
                        dstContainer->addWidget(pallet);

                        ui->label_dst->setText(QStringLiteral("货架#%1").arg(shelvesId));
                        ui->label_src->setText(QStringLiteral("治具#%1").arg(fixtureId));

                        unloading(shelvesId, 1, fixtureId);
                    }
                }
            }
        });
        connect(entry, &Container::sig_addPallet, this, [=](Container* container) {
            if (!m_pallets.contains(container->objectName()) && !container->objectName().contains("fixture")) {
                QLabel* pallet = new QLabel(this);
                m_pallets.insert(container->objectName(), pallet);
                container->addWidget(pallet);
                ui->label_palletsNum->setText(QString::number(m_pallets.size()));
            }
        });
        connect(entry, &Container::sig_removePallet, this, [=](Container* container) {
            if (m_pallets.contains(container->objectName()) && !container->objectName().contains("fixture")) {
                QLabel* pallet = m_pallets[container->objectName()];
                container->removeWidget(pallet);
                m_pallets.remove(container->objectName());
                delete pallet;
                ui->label_palletsNum->setText(QString::number(m_pallets.size()));
            }
        });
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::switchPage(int index)
{
    QVector<QString> widgetName = {"btn_page1", "btn_page2", "btn_page3", "btn_page4", "btn_page5"};
    for (int i = 0; i < widgetName.size(); ++i) {
        QPushButton* btn = ui->widget_10->findChild<QPushButton*>(widgetName[i]);
        if (i == index) {
            btn->setStyleSheet("color: #ffffff; border: none; background-color: #409eff;");
        } else {
            btn->setStyleSheet("color: #ffffff; border: none; background-color: #2d2c32;");
        }
    }
    ui->tabWidget->setCurrentIndex(index);
}

void MainWindow::setLightState(QWidget* parent, const QString& lightName, LightState state)
{
    QLabel* light = parent->findChild<QLabel*>(lightName);
    if (light) {
        switch (state) {
        case LightState::LightGray:
            light->setStyleSheet("border: none; border-radius: 15px;background-color: rgb(97, 97, 97);");
            break;
        case LightState::LightGreen:
            light->setStyleSheet("border: none; border-radius: 15px;background-color: rgb(102, 163, 52);");
            break;
        case LightState::LightRed:
            light->setStyleSheet("border: none; border-radius: 15px;background-color: rgb(192, 88, 88);");
            break;
        default:
            break;
        }
    }
}

void MainWindow::setClickedLightState(QWidget* parent, const QString& lightName, LightState state)
{
    QPushButton* light = parent->findChild<QPushButton*>(lightName);
    if (light) {
        switch (state) {
        case LightState::LightGray:
            light->setStyleSheet("border: none; color: rgb(97, 97, 97); background-color: rgb(97, 97, 97);");
            light->setText("Set");
            break;
        case LightState::LightGreen:
            light->setStyleSheet("border: none; color: rgb(102, 163, 52); background-color: rgb(102, 163, 52);");
            light->setText("Reset");
            break;
        case LightState::LightRed:
            light->setStyleSheet("border: none; color: rgb(192, 128, 128); background-color: rgb(192, 88, 88);");
            break;
        default:
            break;
        }
    }
}

void MainWindow::initTrackUI()
{
    connect(m_trackController.get(), &TrackController::sig_updateData, this, [=](const QVector<quint16>& data) {
        if (data.size() == TRACK_READ_NUMS) {
            int originFinishFlag = data[0];
            int moveFinishFlag = data[1];
            int errorFlag = data[2];
            int heartFlag = data[3];
            originFinishFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackOrigin", LightGreen) : setLightState(ui->widget_lightContainer, "light_trackOrigin", LightGray);
            moveFinishFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackArrived", LightGreen) : setLightState(ui->widget_lightContainer, "light_trackArrived", LightGray);
            errorFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackError", LightRed) : setLightState(ui->widget_lightContainer, "light_trackError", LightGray);
            heartFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackHeart", LightGreen) : setLightState(ui->widget_lightContainer, "light_trackHeart", LightGray);
        } else if (data.size() == TRACK_WRITE_NUMS) {
            int setPosition = data[0];
            int setSpeed = data[2];
            int forwardFlag = data[8];
            int backwardFlag = data[9];
            int originFlag = data[10];
            int moveFlag = data[11];
            int resetFlag = data[12];

            ui->sBox_setPos->setValue(setPosition / 10);        // UI界面上的单位是mm，是实际数值的10分之一
            ui->sBox_setSpeed->setValue(setSpeed / 10);         // UI界面上的单位是mm，是实际数值的10分之一
            forwardFlag == 1? setClickedLightState(ui->widget_37, "btn_trackForward", LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackForward", LightGray);
            backwardFlag == 1? setClickedLightState(ui->widget_37, "btn_trackBackward", LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackBackward", LightGray);
            originFlag == 1? setClickedLightState(ui->widget_37, "btn_trackOrigin", LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackOrigin", LightGray);
            moveFlag == 1? setClickedLightState(ui->widget_37, "btn_trackMove", LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackMove", LightGray);
            resetFlag == 1? setClickedLightState(ui->widget_37, "btn_trackReset", LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackReset", LightGray);
        }
    });
    connect(m_trackController.get(), &TrackController::sig_stateChanged, this, [=](int flag) {
        if (flag == 1) {    // 连接状态
            ui->btn_trackConnect->setText("Disconnect");
            setLightState(ui->widget_lightContainer, "light_trackConnect", LightGreen);
            // 连接成功后每 500ms 读取一次地轨PLC数据
            if (m_timer && !m_timer->isActive()) {
                m_timer->setInterval(500);
                m_timer->start();
            }
        } else if (flag == 0) {     // 未连接状态
            ui->btn_trackConnect->setText("Connect");
            setLightState(ui->widget_lightContainer, "light_trackConnect", LightGray);
            setLightState(ui->widget_lightContainer, "light_trackHeart", LightGray);
            if (m_timer && m_timer->isActive()) {
                m_timer->stop();
            }
        } else {    // 发生故障
            setLightState(ui->widget_lightContainer, "light_trackConnect", LightRed);
        }
    });
    connect(ui->btn_trackConnect, &QPushButton::clicked, this, [=]() {
        if (m_trackController) {
            if (ui->btn_trackConnect->text() == "Connect") {
                QString ip = ui->lineEdit_trackIP->text();
                int port = ui->lineEdit_trackPort->text().toInt();
                m_trackController->connectToPLC(ip, port);
            } else if (ui->btn_trackConnect->text() == "Disconnect") {
                m_trackController->disconnectWithPLC();
            }
        }
    });
    connect(ui->btn_trackReset, &QPushButton::clicked, this, [=]() {
        if (ui->btn_trackReset->text() == "Set") {
            m_trackController->reset(1);
        } else if (ui->btn_trackReset->text() == "Reset") {
            m_trackController->reset(0);
        }
    });
    connect(ui->btn_trackOrigin, &QPushButton::clicked, this, [=]() {
        if (ui->btn_trackOrigin->text() == "Set") {
            m_trackController->setRunToOrigin(1);
        } else if (ui->btn_trackOrigin->text() == "Reset") {
            m_trackController->setRunToOrigin(0);
        }
    });
    connect(ui->btn_trackForward, &QPushButton::clicked, this, [=]() {
        if (ui->btn_trackForward->text() == "Set") {
            m_trackController->setRunForward(1);
        } else if (ui->btn_trackForward->text() == "Reset") {
            m_trackController->setRunForward(0);
        }
    });
    connect(ui->btn_trackBackward, &QPushButton::clicked, this, [=]() {
        if (ui->btn_trackBackward->text() == "Set") {
            m_trackController->setRunBackward(1);
        } else if (ui->btn_trackBackward->text() == "Reset") {
            m_trackController->setRunBackward(0);
        }
    });
    connect(ui->btn_trackMove, &QPushButton::clicked, this, [=]() {
        if (ui->btn_trackMove->text() == "Set") {
            m_trackController->setMove(1);
        } else if (ui->btn_trackMove->text() == "Reset") {
            m_trackController->setMove(0);
        }
    });
    connect(ui->btn_setTrackPos, &QPushButton::clicked, this, [=]() {
        if (ui->btn_setTrackPos->text() == "Apply") {
            int targetPos = ui->sBox_trackPos->value() * 10;         // UI界面上的单位是mm，实际数值应为10倍
            m_trackController->setPosition(targetPos);
        } else if (ui->btn_setTrackPos->text() == "Stop") {
            ui->btn_setTrackPos->setText("Apply");
        }
    });
    connect(ui->btn_setTrackSpeed, &QPushButton::clicked, this, [=]() {
        int targetSpeed = ui->sBox_trackSpeed->value() * 10;        // UI界面上的单位是mm，实际数值应为10倍
        m_trackController->setMovingSpeed(targetSpeed);
    });
    connect(ui->btn_trackRunToTarget, &QPushButton::clicked, this, [=]() {
        if (ui->btn_trackRunToTarget->text() == "Apply") {
            int target = ui->cBox_trackTarget->currentIndex();
            if (target == 0) {
                m_trackController->moveToTarget(SHELVES_POSITION);
            } else if (target == 1) {
                m_trackController->moveToTarget(BOX_1_POSITION);
            } else if (target == 2) {
                m_trackController->moveToTarget(BOX_2_POSITION);
            } else if (target == 3) {
                m_trackController->moveToTarget(BOX_3_POSITION);
            }
            ui->btn_trackRunToTarget->setText("Stop");
        } else if (ui->btn_trackRunToTarget->text() == "Stop") {
            m_trackController->setMove(0);
            ui->btn_trackRunToTarget->setText("Apply");
        }
    });
    connect(ui->btn_trackReset, &QPushButton::clicked, this, [=]() {
        m_trackController->reset(true);
    });
}

void MainWindow::initRobotUI()
{
    connect(m_robotController.get(), &RobotController::sig_updateData, this, [=](const QVector<quint16>& data) {
        data[ROBOT_ORIGIN] == 1 ?
            setLightState(ui->widget_lightContainer, "light_robotOrigin", LightGreen) :
            setLightState(ui->widget_lightContainer, "light_robotOrigin", LightGray);
        data[ROBOT_TRACK1] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack0", LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack0", LightGray);
        data[ROBOT_TRACK2] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack1", LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack1", LightGray);
        data[ROBOT_TRACK3] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack2", LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack2", LightGray);
        data[ROBOT_TRACK4] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack3", LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack3", LightGray);

        int fixtureId = ui->cBox_box->currentIndex();
        int shelvesId = ui->cBox_shelves->currentIndex();
        data[ZWJQ1 + shelvesId] == 1 ?
            setLightState(ui->widget_41, "light_getFromShelvesFinished", LightGreen) :
            setLightState(ui->widget_41, "light_getFromShelvesFinished", LightGray);
        data[ZWJF1 + shelvesId] == 1 ?
            setLightState(ui->widget_41, "light_putToShelvesFinished", LightGreen) :
            setLightState(ui->widget_41, "light_putToShelvesFinished", LightGray);
        data[WKXQ1 + fixtureId] == 1 ?
            setLightState(ui->widget_41, "light_getFromBoxFinished", LightGreen) :
            setLightState(ui->widget_41, "light_getFromBoxFinished", LightGray);
        data[WKXF1 + fixtureId] == 1 ?
            setLightState(ui->widget_41, "light_putToBoxFinished", LightGreen) :
            setLightState(ui->widget_41, "light_putToBoxFinished", LightGray);
    });
    connect(m_robotController.get(), &RobotController::sig_stateChanged, this, [=](int flag) {
        if (flag == 1) {    // 连接状态
            ui->btn_robotConnect->setText("Disconnect");
            setLightState(ui->widget_lightContainer, "light_robotConnect", LightGreen);
            // 连接成功后每隔 1s 读取一次机器人PLC数据
            if (m_timer && !m_timer->isActive()) {
                m_timer->setInterval(1000);
                m_timer->start();
            }
        } else if (flag == 0) {     // 未连接状态
            ui->btn_robotConnect->setText("Connect");
            setLightState(ui->widget_lightContainer, "light_robotConnect", LightGray);
            if (m_timer && m_timer->isActive()) {
                m_timer->stop();
            }
        }
    });
    connect(ui->btn_robotConnect, &QPushButton::clicked, this, [=]() {
        if (m_robotController) {
            if (ui->btn_robotConnect->text() == "Connect") {
                QString ip = ui->lineEdit_robotIP->text();
                int port = ui->lineEdit_robotPort->text().toInt();
                m_robotController->connectToPLC(ip, port);
            } else if (ui->btn_robotConnect->text() == "Disconnect") {
                m_robotController->disconnectWithPLC();
            }
        }
    });
    connect(ui->btn_trackArrived0, &QPushButton::clicked, this, [=]() {
        m_robotController->executeAction(ROBOT_TRACK1);
    });
    connect(ui->btn_trackArrived1, &QPushButton::clicked, this, [=]() {
        m_robotController->executeAction(ROBOT_TRACK2);
    });
    connect(ui->btn_trackArrived2, &QPushButton::clicked, this, [=]() {
        m_robotController->executeAction(ROBOT_TRACK3);
    });
    connect(ui->btn_trackArrived3, &QPushButton::clicked, this, [=]() {
        m_robotController->executeAction(ROBOT_TRACK4);
    });
    connect(ui->btn_getFromShelves, &QPushButton::clicked, this, [=]() {
        int shelvesId = ui->cBox_shelves->currentIndex() + 1;
        m_robotController->getFromShelves(shelvesId);
    });
    connect(ui->btn_putToShelves, &QPushButton::clicked, this, [=]() {
        int shelvesId = ui->cBox_shelves->currentIndex() + 1;
        m_robotController->putToShelves(shelvesId);
    });
    connect(ui->btn_getFromBox, &QPushButton::clicked, this, [=]() {
        int fixtureId = ui->cBox_box->currentIndex() + 1;
        m_robotController->getFromFixture(fixtureId);
    });
    connect(ui->btn_putToBox, &QPushButton::clicked, this, [=]() {
        int fixtureId = ui->cBox_box->currentIndex() + 1;
        m_robotController->putToFixture(fixtureId);
    });
    connect(ui->btn_robotClearBits, &QPushButton::clicked, this, [=]() {
        m_robotController->clearAllBits();
    });
}

void MainWindow::initModuleUI()
{
    // 连接状态发生改变
    connect(m_ioController.get(), &IOController::sig_stateChanged, this, [=](int flag) {
        if (flag == 1) {    // 连接状态
            ui->btn_moduleConnect->setText("Disconnect");
            setLightState(ui->widget_lightContainer, "light_moduleState", LightGreen);
            // 连接成功后每隔 500ms 读取一次IO模块数据
            if (m_timer && !m_timer->isActive()) {
                m_timer->setInterval(500);
                m_timer->start();
            }
        } else if (flag == 0) {     // 未连接状态
            ui->btn_moduleConnect->setText("Connect");
            setLightState(ui->widget_lightContainer, "light_moduleState", LightGray);
            if (m_timer && m_timer->isActive()) {
                m_timer->stop();
            }
        }
    });
    // 通信发生错误
    connect(m_ioController.get(), &IOController::sig_errorOccurred, this, [=]() {
        setLightState(ui->widget_lightContainer, "light_moduleState", LightRed);
    });
    // 通信数据更新
    connect(m_ioController.get(), &IOController::sig_updateData, this, [=](const QVector<quint16>& data) {
        if (data.size() == IO_READ_NUMS) {
            int fixtureId = ui->cBox_fixture->currentIndex() + 1;
            if (fixtureId == 1) {
                data[FIXTURE_1_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightGray);
                data[FIXTURE_1_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightGray);
                if (data[FIXTURE_1_IN1] == 1 && data[FIXTURE_1_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_1_IN1] == 0 && data[FIXTURE_1_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 2) {
                data[FIXTURE_2_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightGray);
                data[FIXTURE_2_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightGray);
                if (data[FIXTURE_2_IN1] == 1 && data[FIXTURE_2_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_2_IN1] == 0 && data[FIXTURE_2_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 3) {
                data[FIXTURE_3_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightGray);
                data[FIXTURE_3_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightGray);
                if (data[FIXTURE_3_IN1] == 1 && data[FIXTURE_3_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_3_IN1] == 0 && data[FIXTURE_3_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 4) {
                data[FIXTURE_4_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightGray);
                data[FIXTURE_4_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightGray);
                if (data[FIXTURE_4_IN1] == 1 && data[FIXTURE_4_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_4_IN1] == 0 && data[FIXTURE_4_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 5) {
                data[FIXTURE_5_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightGray);
                data[FIXTURE_5_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightGray);
                if (data[FIXTURE_5_IN1] == 1 && data[FIXTURE_5_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_5_IN1] == 0 && data[FIXTURE_5_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 6) {
                data[FIXTURE_6_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightGray);
                data[FIXTURE_6_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightGray);
                if (data[FIXTURE_6_IN1] == 1 && data[FIXTURE_6_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_6_IN1] == 0 && data[FIXTURE_6_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            }

            int doorId = ui->cBox_door->currentIndex() + 1;
            if (doorId == 1) {
                data[DOOR_1_IN1] == 1 ? setLightState(ui->widget_53, "light_doorIn1", LightGreen) : setLightState(ui->widget_53, "light_doorIn1", LightGray);
                data[DOOR_1_IN2] == 1 ? setLightState(ui->widget_53, "light_doorIn2", LightGreen) : setLightState(ui->widget_53, "light_doorIn2", LightGray);
                if (data[DOOR_1_IN1] == 0 && data[DOOR_1_IN2] == 1) {
                    ui->label_doorState->setText(QStringLiteral("开门"));
                } else if (data[DOOR_1_IN1] == 1 && data[DOOR_1_IN2] == 0) {
                    ui->label_doorState->setText(QStringLiteral("关门"));
                } else {
                    ui->label_doorState->setText(QStringLiteral("-"));
                }
            } else if (doorId == 2) {
                data[DOOR_2_IN1] == 1 ? setLightState(ui->widget_53, "light_doorIn1", LightGreen) : setLightState(ui->widget_53, "light_doorIn1", LightGray);
                data[DOOR_2_IN2] == 1 ? setLightState(ui->widget_53, "light_doorIn2", LightGreen) : setLightState(ui->widget_53, "light_doorIn2", LightGray);
                if (data[DOOR_2_IN1] == 0 && data[DOOR_2_IN2] == 1) {
                    ui->label_doorState->setText(QStringLiteral("开门"));
                } else if (data[DOOR_2_IN1] == 1 && data[DOOR_2_IN2] == 0) {
                    ui->label_doorState->setText(QStringLiteral("关门"));
                } else {
                    ui->label_doorState->setText(QStringLiteral("-"));
                }
            } else if (doorId == 3) {
                data[DOOR_3_IN1] == 1 ? setLightState(ui->widget_53, "light_doorIn1", LightGreen) : setLightState(ui->widget_53, "light_doorIn1", LightGray);
                data[DOOR_3_IN2] == 1 ? setLightState(ui->widget_53, "light_doorIn2", LightGreen) : setLightState(ui->widget_53, "light_doorIn2", LightGray);
                if (data[DOOR_3_IN1] == 0 && data[DOOR_3_IN2] == 1) {
                    ui->label_doorState->setText(QStringLiteral("开门"));
                } else if (data[DOOR_3_IN1] == 1 && data[DOOR_3_IN2] == 0) {
                    ui->label_doorState->setText(QStringLiteral("关门"));
                } else {
                    ui->label_doorState->setText(QStringLiteral("-"));
                }
            }

            int doorLockId = ui->cBox_doorLock->currentIndex() + 1;
            if (doorLockId == 1) {
                data[DOOR_LOCK_1_IN1] == 1 ? setLightState(ui->widget_53, "light_doorLockIn1", LightGreen) : setLightState(ui->widget_53, "light_doorLockIn1", LightGray);
                data[DOOR_LOCK_1_IN2] == 1 ? setLightState(ui->widget_53, "light_doorLockIn2", LightGreen) : setLightState(ui->widget_53, "light_doorLockIn2", LightGray);
                if (data[DOOR_LOCK_1_IN1] == 1 && data[DOOR_LOCK_1_IN2] == 0) {
                    ui->label_lockState->setText(QStringLiteral("解锁"));
                } else if (data[DOOR_LOCK_1_IN1] == 0 && data[DOOR_LOCK_1_IN2] == 1) {
                    ui->label_lockState->setText(QStringLiteral("关锁"));
                } else {
                    ui->label_lockState->setText(QStringLiteral("-"));
                }
            } else if (doorLockId == 2) {
                data[DOOR_LOCK_2_IN1] == 1 ? setLightState(ui->widget_53, "light_doorLockIn1", LightGreen) : setLightState(ui->widget_53, "light_doorLockIn1", LightGray);
                data[DOOR_LOCK_2_IN2] == 1 ? setLightState(ui->widget_53, "light_doorLockIn2", LightGreen) : setLightState(ui->widget_53, "light_doorLockIn2", LightGray);
                if (data[DOOR_LOCK_2_IN1] == 1 && data[DOOR_LOCK_2_IN2] == 0) {
                    ui->label_lockState->setText(QStringLiteral("解锁"));
                } else if (data[DOOR_LOCK_2_IN1] == 0 && data[DOOR_LOCK_2_IN2] == 1) {
                    ui->label_lockState->setText(QStringLiteral("关锁"));
                } else {
                    ui->label_lockState->setText(QStringLiteral("-"));
                }
            } else if (doorLockId == 3) {
                data[DOOR_LOCK_3_IN1] == 1 ? setLightState(ui->widget_53, "light_doorLockIn1", LightGreen) : setLightState(ui->widget_53, "light_doorLockIn1", LightGray);
                data[DOOR_LOCK_3_IN2] == 1 ? setLightState(ui->widget_53, "light_doorLockIn2", LightGreen) : setLightState(ui->widget_53, "light_doorLockIn2", LightGray);
                if (data[DOOR_LOCK_3_IN1] == 1 && data[DOOR_LOCK_3_IN2] == 0) {
                    ui->label_lockState->setText(QStringLiteral("解锁"));
                } else if (data[DOOR_LOCK_3_IN1] == 0 && data[DOOR_LOCK_3_IN2] == 1) {
                    ui->label_lockState->setText(QStringLiteral("关锁"));
                } else {
                    ui->label_lockState->setText(QStringLiteral("-"));
                }
            }
        } else if (data.size() == IO_WRITE_NUMS) {
            int fixtureId = ui->cBox_fixture->currentIndex() + 1;
            if (fixtureId == 1) {
                data[FIXTURE_1_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGray);
            } else if (fixtureId == 2) {
                data[FIXTURE_2_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGray);
            } else if (fixtureId == 3) {
                data[FIXTURE_3_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGray);
            } else if (fixtureId == 4) {
                data[FIXTURE_4_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGray);
            } else if (fixtureId == 5) {
                data[FIXTURE_5_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGray);
            } else if (fixtureId == 6) {
                data[FIXTURE_6_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightGray);
            }

            int doorId = ui->cBox_door->currentIndex() + 1;
            if (doorId == 1) {
                data[DOOR_1_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_doorOut", LightGray);
            } else if (doorId == 2) {
                data[DOOR_2_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_doorOut", LightGray);
            } else if (doorId == 3) {
                data[DOOR_3_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_doorOut", LightGray);
            }

            int doorLockId = ui->cBox_doorLock->currentIndex() + 1;
            if (doorLockId == 1) {
                data[DOOR_LOCK_1_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorLockOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_doorLockOut", LightGray);
            } else if (doorLockId == 2) {
                data[DOOR_LOCK_2_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorLockOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_doorLockOut", LightGray);
            } else if (doorLockId == 3) {
                data[DOOR_LOCK_3_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorLockOut", LightGreen) : setClickedLightState(ui->widget_53, "btn_doorLockOut", LightGray);
            }
        }
    });
    connect(ui->btn_moduleConnect, &QPushButton::clicked, this, [=]() {
        if (m_ioController) {
            if (ui->btn_moduleConnect->text() == "Connect") {
                QString ip = ui->lineEdit_moduleIP->text();
                int port = ui->lineEdit_modulePort->text().toInt();
                m_ioController->connectToPLC(ip, port);
            } else if (ui->btn_moduleConnect->text() == "Disconnect") {
                m_ioController->disconnectWithPLC();
            }
        }
    });
    connect(ui->btn_fixtureOut, &QPushButton::clicked, this, [=]() {
        if (ui->btn_fixtureOut->text() == "Set") {
            int fixtureId = ui->cBox_fixture->currentIndex() + 1;
            m_ioController->fixtureSet(fixtureId, 1);
        } else if (ui->btn_fixtureOut->text() == "Reset") {
            int fixtureId = ui->cBox_fixture->currentIndex() + 1;
            m_ioController->fixtureSet(fixtureId, 0);
        }
    });
    connect(ui->btn_doorOut, &QPushButton::clicked, this, [=]() {
        if (ui->btn_doorOut->text() == "Set") {
            int doorId = ui->cBox_door->currentIndex() + 1;
            m_ioController->boxOpen(doorId);
        } else if (ui->btn_doorOut->text() == "Reset") {
            int doorId = ui->cBox_door->currentIndex() + 1;
            m_ioController->boxClose(doorId);
        }
    });
    connect(ui->btn_doorLockOut, &QPushButton::clicked, this, [=]() {
        if (ui->btn_doorLockOut->text() == "Set") {
            int doorId = ui->cBox_doorLock->currentIndex() + 1;
            m_ioController->boxUnlocking(doorId);
        } else if (ui->btn_doorLockOut->text() == "Reset") {
            int doorId = ui->cBox_doorLock->currentIndex() + 1;
            m_ioController->boxLocking(doorId);
        }
    });
}

void MainWindow::loading(int shelvesId, int boxId, int fixtureId)
{
    long long count = 0;
    // // 0. 检查指定温控箱是否在工作，内部是否有托盘，门是否为锁紧状态，机械臂是否为原点状态
    ui->textBrowser->clear();
    ui->textBrowser->append(QString("正在将托盘从 %1 号货架运至 %2 号检测位...").arg(shelvesId).arg(fixtureId));

    m_robotController->clearAllBits();

    // 1. 温控箱开锁
    if (m_stopProcess) {
        qDebug() << "process stop";
        return;
    }
    m_ioController->boxUnlocking(boxId);
    // 启动一个计数器
    count = 0;

    // 2. 等待温控箱开锁完成信号，温控箱开门
    while (m_ioController->getBoxLockState(boxId) != LOCK_STATE::UNLOCKING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxUnlocking(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 已开锁").arg(boxId));
    m_ioController->boxOpen(boxId);
    count = 0;

    // 3. 等待温控箱开门完成信号，治具上升
    while (m_ioController->getBoxOpenState(boxId) != DOOR_STATE::OPENING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxOpen(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 门已打开").arg(boxId));
    m_ioController->fixtureSet(fixtureId, 0);
    count = 0;

    // 4. 等待治具上升完成信号，给机械臂下发叉取托盘命令
    while (m_ioController->getFixtureState(fixtureId) != FIXTURE_STATE::UP) {
        count++;
        if (count >= 1000000) {
            m_ioController->fixtureSet(fixtureId, 0);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("治具 %1 已抬起").arg(fixtureId));
    m_robotController->getFromShelves(shelvesId);

    // 5. 等待机械臂返回地轨移动请求，移动地轨至货架点位
    while (!m_robotController->requireTrackMove(SHELVES_POSITION)) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append("地轨移动中...");
    m_trackController->moveToTarget(SHELVES_POSITION);

    // 6. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append("地轨已移动至货架点位");
    ui->textBrowser->append(QString("正在从 %1 号货架处夹取托盘...").arg(shelvesId));
    m_trackController->setMove(false);
    m_robotController->trackMoveArrived(SHELVES_POSITION);

    // 7. 等待机械臂返回叉取完成信号，给机械臂下发放置命令
    while (!m_robotController->resultOfGetFromShelves(shelvesId)) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append("托盘夹取完成");
    m_robotController->putToFixture(fixtureId);

    // 8. 等待机械臂返回地轨移动请求，移动地轨至检测箱点位
    if (boxId == 1) {
        while (!m_robotController->requireTrackMove(BOX_1_POSITION)) {
            QCoreApplication::processEvents();
            if (m_stopProcess) {
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_1_POSITION);
    } else if (boxId == 2) {
        while (!m_robotController->requireTrackMove(BOX_2_POSITION)) {
            QCoreApplication::processEvents();
            if (m_stopProcess) {
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_2_POSITION);
    } else if (boxId == 3) {
        while (!m_robotController->requireTrackMove(BOX_3_POSITION)) {
            QCoreApplication::processEvents();
            if (m_stopProcess) {
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_3_POSITION);
    }
    ui->textBrowser->append("地轨移动中...");

    // 9. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    if (boxId == 1) {
        m_robotController->trackMoveArrived(BOX_1_POSITION);
    } else if (boxId == 2) {
        m_robotController->trackMoveArrived(BOX_2_POSITION);
    } else if (boxId == 3) {
        m_robotController->trackMoveArrived(BOX_3_POSITION);
    }
    ui->textBrowser->append(QString("地轨已移动至 %1 号工位").arg(boxId));
    ui->textBrowser->append(QString("正在向 %1 号治具处放置托盘...").arg(fixtureId));

    // 10. 等待机械臂返回放置完成信号，机械臂信号清零，治具下压
    while (!m_robotController->resultOfPutToFixture(fixtureId)) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    m_robotController->clearAllBits();
    m_ioController->fixtureSet(fixtureId, 1);
    count = 0;

    // 11. 等待治具下压完成信号，温控箱关门
    while (m_ioController->getFixtureState(fixtureId) != FIXTURE_STATE::DOWN) {
        count++;
        if (count >= 1000000) {
            m_ioController->fixtureSet(fixtureId, 1);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("治具 %1 已下压").arg(fixtureId));
    m_ioController->boxClose(boxId);
    count = 0;

    // 13. 等待温控箱关门完成信号，温控箱锁门
    while (m_ioController->getBoxOpenState(boxId) != DOOR_STATE::CLOSING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxClose(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 门已关闭").arg(boxId));
    m_ioController->boxLocking(boxId);
    count = 0;

    // 14. 等待温控箱锁门完成信号，地轨回原点
    while (m_ioController->getBoxLockState(boxId) != LOCK_STATE::LOCKING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxLocking(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 已上锁").arg(boxId));
    m_trackController->moveToTarget(ORIGIN_POSITION);
    ui->textBrowser->append("地轨回原点中...");

    // 15. 等待地轨回原点完成，开始检测
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    ui->textBrowser->append("放置完成！开始检测...");
}

void MainWindow::unloading(int shelvesId, int boxId, int fixtureId)
{
    // // 0. 检查指定温控箱是否在工作，内部是否有托盘，门是否为锁紧状态，机械臂是否为原点状态

    long long count = 0;

    ui->textBrowser->clear();
    ui->textBrowser->append(QString("正在将托盘从 %1 号治具运至 %2 号置物架...").arg(fixtureId).arg(shelvesId));

    m_robotController->clearAllBits();

    // 1. 治具上升
    m_ioController->fixtureSet(fixtureId, 0);
    count = 0;

    // 2. 等待治具上升完成，温控箱解锁
    while (m_ioController->getFixtureState(fixtureId) != FIXTURE_STATE::UP) {
        count++;
        if (count >= 1000000) {
            m_ioController->fixtureSet(fixtureId, 0);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("治具 %1 已抬起").arg(fixtureId));
    m_ioController->boxUnlocking(boxId);
    count = 0;

    // 3. 等待温控箱解锁完成，温控箱开门
    while (m_ioController->getBoxLockState(boxId) != LOCK_STATE::UNLOCKING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxUnlocking(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 已解锁").arg(boxId));
    m_ioController->boxOpen(boxId);
    count = 0;

    // 4. 等待温控箱开门完成，给机械臂发送叉取托盘命令
    while (m_ioController->getBoxOpenState(boxId) != DOOR_STATE::OPENING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxOpen(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 门已打开").arg(boxId));
    m_robotController->getFromFixture(fixtureId);

    // 5. 等待机械臂返回地轨移动请求，移动地轨至检测箱工位
    if (boxId == 1) {
        while (!m_robotController->requireTrackMove(BOX_1_POSITION)) {
            QCoreApplication::processEvents();
            if (m_stopProcess) {
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_1_POSITION);
    } else if (boxId == 2) {
        while (!m_robotController->requireTrackMove(BOX_2_POSITION)) {
            QCoreApplication::processEvents();
            if (m_stopProcess) {
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_2_POSITION);
    } else if (boxId == 3) {
        while (!m_robotController->requireTrackMove(BOX_3_POSITION)) {
            QCoreApplication::processEvents();
            if (m_stopProcess) {
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_3_POSITION);
    }
    ui->textBrowser->append("地轨移动中...");

    // 6. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    if (boxId == 1) {
        m_robotController->trackMoveArrived(BOX_1_POSITION);
    } else if (boxId == 2) {
        m_robotController->trackMoveArrived(BOX_2_POSITION);
    } else if (boxId == 3) {
        m_robotController->trackMoveArrived(BOX_3_POSITION);
    }
    ui->textBrowser->append(QString("地轨已移动至 %1 号工位").arg(boxId));
    ui->textBrowser->append(QString("正在从 %1 号治具处叉取托盘...").arg(fixtureId));

    // 7. 等待机械臂返回叉取完成信号，给机械臂下发放置命令
    while (!m_robotController->resultOfGetFromFixture(fixtureId)) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append("托盘夹取完成");
    m_robotController->putToShelves(shelvesId);

    // 8. 等待机械臂返回地轨移动请求，移动地轨至货架位
    while (!m_robotController->requireTrackMove(SHELVES_POSITION)) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append("地轨移动中...");
    m_trackController->moveToTarget(SHELVES_POSITION);

    // 9. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    m_robotController->trackMoveArrived(SHELVES_POSITION);
    ui->textBrowser->append("地轨已移动至货架点位");
    ui->textBrowser->append(QString("正在向 %1 号货架处放置托盘...").arg(shelvesId));

    // 10. 等待机械臂返回放置完成信号，机械臂信号清零，温控箱关门
    while (!m_robotController->resultOfPutToShelves(shelvesId)) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    m_robotController->clearAllBits();
    m_ioController->boxClose(boxId);
    count = 0;

    // 11. 等待温控箱关门完成信号，温控箱锁门
    while (m_ioController->getBoxOpenState(boxId) != DOOR_STATE::CLOSING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxClose(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 门已关闭").arg(boxId));
    m_ioController->boxLocking(boxId);
    count = 0;

    // 12. 等待温控箱锁门完成信号，地轨回原点
    while (m_ioController->getBoxLockState(boxId) != LOCK_STATE::LOCKING) {
        count++;
        if (count >= 1000000) {
            m_ioController->boxLocking(boxId);
            count = 0;
        }
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString("检测箱 %1 已上锁").arg(boxId));
    m_trackController->moveToTarget(ORIGIN_POSITION);
    ui->textBrowser->append("地轨回原点中...");

    // 13. 等待地轨回原点完成，结束
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_stopProcess) {
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    ui->textBrowser->append("放置完成！");
}
