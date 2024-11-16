#include "MainWindow.h"
#include "./ui_MainWindow.h"


extern cv::Mat grabImg1;
extern HWND hWnd1;
extern cv::Mat grabImg2;
extern HWND hWnd2;

extern cv::Mat grabImg3;
// extern QImage* QmyImage = new QImage();
extern QImage* QmyImage ;

extern int g_warningSignal;
extern int g_enterSignal;
extern int g_leaveSignal;

CMvCamera camera;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_robotController(std::make_unique<RobotController>()),
    m_trackController(std::make_unique<TrackController>()),
    m_ioController(std::make_unique<IOController>()),
    m_comTimer(new QTimer()),
    m_clockTimer(new QTimer()),
    m_detectTimer(new QTimer()),
    m_ioComTimer(new QTimer()),
    m_taskTimer(new QTimer()),
    m_taskList(new TaskListModel()),
    m_stopTimer(new QTimer()),
    m_processState(ProcessState::STOPPED)
{
    ui->setupUi(this);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // 设置窗口标题
    this->setWindowTitle(u8"电池自动检测上下料系统");
    // 默认全屏显示
    this->showMaximized();
    ui->tabWidget->tabBar()->hide();
    // 默认显示"操作界面"页面
    switchPage(0);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    connect(ui->btn_capture, &QPushButton::clicked, this, [=]() {
        // 每秒刷新一次末端相机监控图像
        on_pb_single_clicked();
        // 托盘对比
        double result = camera.DetectionP(grabImg3, 1);
        //通过以下代码，把opencv的Mat转换为QImage，然后在Qt的QLabel控件上进行显示
        cv::imwrite("C:/Users/panrou/Desktop/RobotProject/SDK/img0.jpg",grabImg3);
        QImage qgrabImg3 = QImage((const unsigned char*)(grabImg3.data), grabImg3.cols, grabImg3.rows, grabImg3.cols * grabImg3.channels(), QImage::Format_RGB888);   // Mat为BGR颜色模式;
        qgrabImg3 = qgrabImg3.rgbSwapped();
        // std::cout << qgrabImg3.width() << " " << qgrabImg3.height() << std::endl;
        ui->label_cameraView->setPixmap(QPixmap::fromImage(qgrabImg3));

        if (result > 0.8) {
            ui->label_result->setText("OK");
            ui->label_result->setStyleSheet("color: green;");
        } else {
            ui->label_result->setText("NG");
            ui->label_result->setStyleSheet("color: red;");
        }
    });

    connect(ui->btn_capture2, &QPushButton::clicked, this, [=]() {
        // 每秒刷新一次末端相机监控图像
        on_pb_single_clicked();
        // 托盘对比
        double result = camera.DetectionP(grabImg3, 2);
        //通过以下代码，把opencv的Mat转换为QImage，然后在Qt的QLabel控件上进行显示
        cv::imwrite("C:/Users/panrou/Desktop/RobotProject/SDK/img0.jpg",grabImg3);
        QImage qgrabImg3 = QImage((const unsigned char*)(grabImg3.data), grabImg3.cols, grabImg3.rows, grabImg3.cols * grabImg3.channels(), QImage::Format_RGB888);   // Mat为BGR颜色模式;
        qgrabImg3 = qgrabImg3.rgbSwapped();
        // std::cout << qgrabImg3.width() << " " << qgrabImg3.height() << std::endl;
        ui->label_cameraView->setPixmap(QPixmap::fromImage(qgrabImg3));
        if (result > 0.8) {
            ui->label_result->setText("OK");
            ui->label_result->setStyleSheet("color: green;");
        } else {
            ui->label_result->setText("NG");
            ui->label_result->setStyleSheet("color: red;");
        }
    });

    connect(ui->btn_addChannel, &QPushButton::clicked, this, [=]() {
        ui->tableWidget_channelList->insertRow(ui->tableWidget_channelList->rowCount());
        QTableWidgetItem* item;
        item = new QTableWidgetItem("0");
        ui->tableWidget_channelList->setItem(ui->tableWidget_channelList->rowCount() - 1, 0, item);
        item = new QTableWidgetItem("0");
        ui->tableWidget_channelList->setItem(ui->tableWidget_channelList->rowCount() - 1, 1, item);
        item = new QTableWidgetItem("0");
        ui->tableWidget_channelList->setItem(ui->tableWidget_channelList->rowCount() - 1, 2, item);
        QPushButton* btn = new QPushButton("-");
        btn->setStyleSheet("QPushButton {\
                                background-color: rgb(85, 85, 127);\
                                color: #fff;\
                            }\
                            \
                            QPushButton::pressed {\
                                background-color: rgb(69, 69, 103);\
                                color: #fff;\
                            }");
        ui->tableWidget_channelList->setCellWidget(ui->tableWidget_channelList->rowCount() - 1, 3, btn);
    });
    connect(ui->btn_refreshChannel, &QPushButton::clicked, this, [=]() {
        int shelvesID = ui->cBox_shelvesID->currentIndex() + 1;
        int boxID = ui->cBox_boxID->currentIndex() + 1;
        int fixtureID = 0;
        if (ui->cBox_fixtureID->currentIndex() == 0) {
            if (boxID == 1) {
                fixtureID = 1;
            } else if (boxID == 2) {
                fixtureID = 3;
            } else if (boxID == 3) {
                fixtureID = 5;
            }
        } else if (ui->cBox_fixtureID->currentIndex() == 1) {
            if (boxID == 1) {
                fixtureID = 2;
            } else if (boxID == 2) {
                fixtureID = 4;
            } else if (boxID == 3) {
                fixtureID = 6;
            }
        }
        loadChannelFile(fixtureID);
    });

    // "机器人外部启动"按键
    connect(ui->btn_robotInit, &QPushButton::clicked, this, [=]() {
        // 外部伺服上电
        m_robotController->applyElec();
        int count = 500000;
        while (count--) {
            QApplication::processEvents();
        }
        // 主程序调用
        m_robotController->runProcess();
        count = 500000;
        while (count--) {
            QApplication::processEvents();
        }
        // 外部启动
        m_robotController->runRobot();
    });

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

    connect(ui->btn_page3, &QPushButton::clicked, this, [=]() {
        //监控1
        hWnd1 = (HWND)(ui->label_monitorView1->winId());
        watch_monitor1(0, 0);
        //通过以下代码，把opencv的Mat转换为QImage，然后在Qt的QLabel控件上进行显示
        QImage qgrabImg1 = QImage((const unsigned char*)(grabImg1.data), grabImg1.cols, grabImg1.rows, grabImg1.cols * grabImg1.channels(), QImage::Format_RGB888);   // Mat为BGR颜色模式;
        qgrabImg1 = qgrabImg1.rgbSwapped();
        ui->label_monitorView1->setPixmap(QPixmap::fromImage(qgrabImg1));

        //监控2
        hWnd2 = (HWND)(ui->label_monitorView2->winId());
        watch_monitor2(0, 0);
        //通过以下代码，把opencv的Mat转换为QImage，然后在Qt的QLabel控件上进行显示
        QImage qgrabImg2 = QImage((const unsigned char*)(grabImg2.data), grabImg2.cols, grabImg2.rows, grabImg2.cols * grabImg2.channels(), QImage::Format_RGB888);   // Mat为BGR颜色模式;
        qgrabImg2 = qgrabImg2.rgbSwapped();
        ui->label_monitorView2->setPixmap(QPixmap::fromImage(qgrabImg2));

        // //末端相机
        // // on_pb_find_clicked();
        // // on_pb_open_clicked();
        // on_pb_single_clicked();
        // //通过以下代码，把opencv的Mat转换为QImage，然后在Qt的QLabel控件上进行显示
        // QImage qgrabImg3 = QImage((const unsigned char*)(grabImg3.data), grabImg3.cols, grabImg3.rows, grabImg3.cols * grabImg3.channels(), QImage::Format_RGB888);   // Mat为BGR颜色模式;
        // qgrabImg3 = qgrabImg3.rgbSwapped();
        // ui->label_cameraView->setPixmap(QPixmap::fromImage(qgrabImg3));

        // *QmyImage = (*QmyImage).scaled(ui->label_cameraView->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        // ui->label_cameraView->setPixmap(QPixmap::fromImage(*QmyImage));
        // delete QmyImage;
    });

    // 隐藏"数据管理"页面
    ui->btn_page4->hide();
    // 隐藏"日志记录"页面
    ui->btn_page5->hide();
    ui->tab->hide();
    ui->tab_3->hide();

    connect(m_taskList, &QAbstractTableModel::dataChanged, this, [=](const QModelIndex&, const QModelIndex&, const QVector<int>&) {
        ui->tableView->viewport()->update();
    });
    ui->tableView->setModel(m_taskList);
    ui->tableView->setColumnWidth(0, 100);
    ui->tableView->setColumnWidth(1, 100);
    ui->tableView->setColumnWidth(2, 100);
    ui->tableView->setColumnWidth(3, 100);
    ui->tableView->setColumnWidth(4, 100);
    ui->tableView->setColumnWidth(5, 230);
    ui->tableView->setColumnWidth(6, 230);

    ui->tableWidget_channelList->setColumnWidth(0, 200);
    ui->tableWidget_channelList->setColumnWidth(1, 200);
    ui->tableWidget_channelList->setColumnWidth(2, 200);

    // 定时读取数据
    connect(m_comTimer, &QTimer::timeout, this, [=]() {
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
    });

    connect(m_ioComTimer, &QTimer::timeout, this, [=]() {
        // 每秒读取一次IO模块数据
        if (m_ioController && m_ioController->isConnected()) {
            static int last = 1;
            m_ioController->requestFeedback(last);
            last = !last;
        }
    });

    // 上料按键
    connect(ui->btn_load, &QPushButton::clicked, this, [=]() {
        int shelvesId = ui->cBox_shelvesId->currentIndex() + 1;      // 1 - 12
        int boxId = ui->cBox_boxId->currentIndex() + 1;              // 1 - 3
        int fixtureId = ui->cBox_fixtureId->currentIndex() + 1;      // 1 - 6
        m_processState = ProcessState::RUNING;
        ui->btn_pauseProcess->setText("Pause");
        loading(shelvesId, boxId, fixtureId);
    });
    // 下料按键
    connect(ui->btn_unload, &QPushButton::clicked, this, [=]() {
        int shelvesId = ui->cBox_shelvesId->currentIndex() + 1;      // 1 - 12
        int boxId = ui->cBox_boxId->currentIndex() + 1;              // 1 - 3
        int fixtureId = ui->cBox_fixtureId->currentIndex() + 1;      // 1 - 6
        m_processState = ProcessState::RUNING;
        ui->btn_pauseProcess->setText("Pause");
        unloading(shelvesId, boxId, fixtureId);
    });
    // 停止按键
    connect(ui->btn_stopProcess, &QPushButton::clicked, this, [=]() {
        m_processState = ProcessState::STOPPED;
    });
    // 暂停按键
    connect(ui->btn_pauseProcess, &QPushButton::clicked, this, [=]() {
        if (ui->btn_pauseProcess->text() == "Pause") {      // 如果当前为运行状态
            // 暂停流程
            m_processState = ProcessState::PAUSED;
            // 外部暂停机器人
            m_robotController->pauseRobot();
            ui->btn_pauseProcess->setText("Resume");
        } else if (ui->btn_pauseProcess->text() == "Resume") {      // 如果当前为暂停状态
            // 恢复流程
            m_processState = ProcessState::RUNING;
            // 外部启动机器人
            m_robotController->runRobot();
            ui->btn_pauseProcess->setText("Pause");
            // 全局变量清零以接收下一次急停信号
            g_warningSignal = 0;
        }
    });

    // 弹出选择工步文件对话框
    connect(ui->btn_chooseFile, &QPushButton::clicked, this, [=]() {
        QString filename = QFileDialog::getOpenFileName(
                               nullptr,
                               tr("Open File"),
                               "\\\\192.168.1.250/Template/",    // 局域网内的共享文件夹
                               tr("All Files (*)")        // 文件过滤器
                               ).split('/').back();       // 获取文件名（不带路径）

        if (!filename.isEmpty()) {
            ui->lineEdit_stepFilename->setText(filename);
            ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
        }
    });

    // "添加任务"按键
    connect(ui->btn_execute, &QPushButton::clicked, this, [=]() {
        int shelvesID = ui->cBox_shelvesID->currentIndex() + 1;
        int boxID = ui->cBox_boxID->currentIndex() + 1;
        int fixtureID = 0;
        if (ui->cBox_fixtureID->currentIndex() == 0) {
            if (boxID == 1) {
                fixtureID = 1;
            } else if (boxID == 2) {
                fixtureID = 3;
            } else if (boxID == 3) {
                fixtureID = 5;
            }
        } else if (ui->cBox_fixtureID->currentIndex() == 1) {
            if (boxID == 1) {
                fixtureID = 2;
            } else if (boxID == 2) {
                fixtureID = 4;
            } else if (boxID == 3) {
                fixtureID = 6;
            }
        }

        std::string deviceID = ui->tableWidget_channelList->item(0, 0)->text().toStdString();
        std::string cellID = ui->tableWidget_channelList->item(0, 1)->text().toStdString();
        std::string channelID = ui->tableWidget_channelList->item(0, 2)->text().toStdString();
        // std::string deviceID = ui->cBox_deviceID->currentText().toStdString();
        // std::string cellID = ui->cBox_unitID->currentText().toStdString();
        // std::string channelID = ui->cBox_channelID->currentText().toStdString();
        std::string filename = ui->lineEdit_stepFilename->text().toStdString();

        QDateTime specificTime = ui->dateTimeEdit->dateTime();

        // 在任务列表中添加一行数据
        Task task;
        task.shelvesID = shelvesID;
        task.fixtureID = fixtureID;
        task.deviceID = std::stoi(deviceID);
        task.cellID = std::stoi(cellID);
        task.channelID = std::stoi(channelID);
        task.stepFilename = filename;
        task.date = specificTime;
        task.status = TaskState::WAITING;
        m_taskList->addData(task);
    });

    // 用于检测急停信号
    connect(m_stopTimer, &QTimer::timeout, this, [=]() {         
        // 如果触发人员入侵信号
        if (g_warningSignal == 1) {
            if (ui->checkBox_emergencyStop->isChecked() && m_processState == ProcessState::RUNING) {
                // 暂停流程
                m_processState = ProcessState::PAUSED;
                // 外部暂停机器人
                m_robotController->pauseRobot();
                ui->btn_pauseProcess->setText("Resume");
                // 报警灯闪烁鸣笛报警
                m_ioController->setWarningLight(true);
                // 报警五秒后关闭报警
                QTimer::singleShot(5000, [=]() {
                    m_ioController->setWarningLight(false);
                });
            }
        }
    });

    // 用于实时更新时间
    connect(m_clockTimer, &QTimer::timeout, this, [=]() {
        // 获取当前时间
        QDateTime currentDateTime = QDateTime::currentDateTime();
        // 更新当前时间
        ui->label_currTime->setText(currentDateTime.toString("yyyy-MM-dd HH:mm:ss"));
        // 计算软件已运行时长
        ui->label_runningTime->setText(QTime(0, 0).addMSecs(m_startingTime.msecsTo(currentDateTime)).toString("hh:mm:ss"));
    });

    // 获取当前时间作为开机时间
    m_startingTime = QDateTime::currentDateTime();
    ui->label_startingTime->setText(m_startingTime.toString("yyyy-MM-dd\n HH:mm:ss"));

    connect(m_detectTimer, &QTimer::timeout, this, [=]() {
        if (m_taskList->rowCount() == 0) {
            return;
        }

        std::vector<std::vector<int>> list;
        // 检查任务队列，将所有要查询的通道号保存下来
        for (int i = 0; i < m_taskList->rowCount(); ++i) {
            Task task = m_taskList->getData(i);
            // 检查正在运行中的通道
            if (task.status == TaskState::RUNNING) {
                list.push_back({task.deviceID, task.cellID, task.channelID});
            }
        }
        // 获取检测状态
        std::vector<std::string> status = getDetectionStatus(list);
        // 再次检查任务队列，将所有已完成的通道号设为准备卸料状态
        for (int i = 0, j = 0; i < m_taskList->rowCount(); ++i) {
            Task task = m_taskList->getData(i);
            if (task.status == TaskState::RUNNING) {
                if (status[j] == "finish" || status[j] == "stop") {
                    task.status = TaskState::READY_UNLOAD;
                    m_taskList->modifyData(i, task);
                    j++;
                }
            }
        }
    });

    connect(m_taskTimer, &QTimer::timeout, this, [=]() {
        // 获取当前时间
        QDateTime currTime = QDateTime::currentDateTime();
        // 检测任务队列
        for (int i = 0; i < m_taskList->rowCount(); ++i) {
            Task task = m_taskList->getData(i);
            // 检查时间有没有到(当前时间超过设定时间则认为已到达指定时间)
            if (task.status == TaskState::WAITING && currTime.msecsTo(task.date) <= 0) {
                task.status = TaskState::READY_LOAD;
                m_taskList->modifyData(i, task);
            }
        }
        // 再次检查任务队列
        for (int i = 0; i < m_taskList->rowCount(); ++i) {
            Task task = m_taskList->getData(i);
            if (task.status == TaskState::READY_LOAD) {
                int boxID = 0;
                if (task.fixtureID == 1 || task.fixtureID == 2) {
                    boxID = 1;
                } else if (task.fixtureID == 3 || task.fixtureID == 4) {
                    boxID = 2;
                } else if (task.fixtureID == 5 || task.fixtureID == 6) {
                    boxID = 3;
                }
                // 切换状态为上料中，更新表格
                task.status = TaskState::LOADING;
                m_taskList->modifyData(i, task);

                // 开始上料
                m_processState = ProcessState::RUNING;
                ui->btn_pauseProcess->setText("Pause");
                ui->textBrowser->append(u8"启动上料");
                loading(task.shelvesID, boxID, task.fixtureID);
                // 上料完成，开始检测
                if (m_processState == ProcessState::RUNING) {
                    int ret = startDetection(std::to_string(task.deviceID), std::to_string(task.cellID), std::to_string(task.channelID), task.stepFilename);
                    if (ret == -1) {
                        std::cout << "启动检测失败" << std::endl;
                    } else if (ret == 1) {
                        std::cout << "启动检测成功" << std::endl;
                    }
                    ui->textBrowser->append(u8"开始检测...");

                    task.status = TaskState::RUNNING;
                    m_taskList->modifyData(i, task);
                }
            } else if (task.status == TaskState::READY_UNLOAD) {
                // 切换状态为卸料中，更新表格
                task.status = TaskState::UNLOADING;
                m_taskList->modifyData(i, task);
                // 开始卸料
                int boxID = 0;
                if (task.fixtureID == 1 || task.fixtureID == 2) {
                    boxID = 1;
                } else if (task.fixtureID == 3 || task.fixtureID == 4) {
                    boxID = 2;
                } else if (task.fixtureID == 5 || task.fixtureID == 6) {
                    boxID = 3;
                }
                m_processState = ProcessState::RUNING;
                ui->btn_pauseProcess->setText("Pause");
                unloading(task.shelvesID, boxID, task.fixtureID);
                // 下料完成，更新表格
                task.status = TaskState::FINISHING;
                m_taskList->modifyData(i, task);
            }
        }
    });

    initTrackUI();
    initRobotUI();
    initModuleUI();
    //监控设备初始化
    open_monitor1(0,0);
    open_monitor2(0,0);
    MonitorinitWidget();
    on_pb_find_clicked();
    on_pb_open_clicked();

    // 启动时间更新时钟
    if (m_clockTimer) {
        m_clockTimer->setInterval(1000);    // 每1秒钟更新一次
        m_clockTimer->start();
    }

    // 启动检测完成轮询时钟
    if (m_detectTimer) {
        m_detectTimer->setInterval(1000 * 60 * 1);      // 每1分钟检查一次
        m_detectTimer->start();
    }

    // 启动任务执行时钟
    if (m_taskTimer) {
        m_taskTimer->setInterval(1000 * 60);    // 每1分钟检查一次
        m_taskTimer->start();
    }

    // 启动急停检测时钟
    if (m_stopTimer) {
        m_stopTimer->setInterval(500);
        m_stopTimer->start();
    }

    // 自动连接地轨、机器人以及IO模块
    emit ui->btn_trackConnect->clicked(true);
    emit ui->btn_moduleConnect->clicked(true);
    emit ui->btn_robotConnect->clicked(true);
}

MainWindow::~MainWindow()
{
    m_processState = ProcessState::STOPPED;

    if (m_robotController && m_robotController->isConnected()) {
        m_robotController->disconnectWithPLC();
    }
    if (m_trackController && m_trackController->isConnected()) {
        m_trackController->disconnectWithPLC();
    }
    if (m_ioController && m_ioController->isConnected()) {
        m_ioController->disconnectWithPLC();
    }

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_processState = ProcessState::STOPPED;
    QMainWindow::closeEvent(event);
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
            originFinishFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackOrigin", LightState::LightGreen) : setLightState(ui->widget_lightContainer, "light_trackOrigin", LightState::LightGray);
            moveFinishFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackArrived", LightState::LightGreen) : setLightState(ui->widget_lightContainer, "light_trackArrived", LightState::LightGray);
            errorFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackError", LightState::LightRed) : setLightState(ui->widget_lightContainer, "light_trackError", LightState::LightGray);
            heartFlag == 1 ? setLightState(ui->widget_lightContainer, "light_trackHeart", LightState::LightGreen) : setLightState(ui->widget_lightContainer, "light_trackHeart", LightState::LightGray);
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
            forwardFlag == 1? setClickedLightState(ui->widget_37, "btn_trackForward", LightState::LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackForward", LightState::LightGray);
            backwardFlag == 1? setClickedLightState(ui->widget_37, "btn_trackBackward", LightState::LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackBackward", LightState::LightGray);
            originFlag == 1? setClickedLightState(ui->widget_37, "btn_trackOrigin", LightState::LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackOrigin", LightState::LightGray);
            moveFlag == 1? setClickedLightState(ui->widget_37, "btn_trackMove", LightState::LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackMove", LightState::LightGray);
            resetFlag == 1? setClickedLightState(ui->widget_37, "btn_trackReset", LightState::LightGreen) :
                setClickedLightState(ui->widget_37, "btn_trackReset", LightState::LightGray);
        }
    });
    connect(m_trackController.get(), &TrackController::sig_stateChanged, this, [=](int flag) {
        if (flag == 1) {    // 连接状态
            ui->btn_trackConnect->setText("Disconnect");
            setLightState(ui->widget_lightContainer, "light_trackConnect", LightState::LightGreen);
            // 连接成功后每 500ms 读取一次地轨PLC数据
            if (m_comTimer && !m_comTimer->isActive()) {
                m_comTimer->setInterval(500);
                m_comTimer->start();
            }
        } else if (flag == 0) {     // 未连接状态
            ui->btn_trackConnect->setText("Connect");
            setLightState(ui->widget_lightContainer, "light_trackConnect", LightState::LightGray);
            setLightState(ui->widget_lightContainer, "light_trackHeart", LightState::LightGray);
            if (m_comTimer && m_comTimer->isActive()) {
                m_comTimer->stop();
            }
        } else {    // 发生故障
            setLightState(ui->widget_lightContainer, "light_trackConnect", LightState::LightRed);
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
            setLightState(ui->widget_lightContainer, "light_robotOrigin", LightState::LightGreen) :
            setLightState(ui->widget_lightContainer, "light_robotOrigin", LightState::LightGray);
        data[ROBOT_TRACK1] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack0", LightState::LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack0", LightState::LightGray);
        data[ROBOT_TRACK2] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack1", LightState::LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack1", LightState::LightGray);
        data[ROBOT_TRACK3] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack2", LightState::LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack2", LightState::LightGray);
        data[ROBOT_TRACK4] == 1 ?
            setClickedLightState(ui->widget_41, "btn_requireTrack3", LightState::LightGreen) :
            setClickedLightState(ui->widget_41, "btn_requireTrack3", LightState::LightGray);

        int fixtureId = ui->cBox_box->currentIndex();
        int shelvesId = ui->cBox_shelves->currentIndex();
        data[ZWJQ1 + shelvesId] == 1 ?
            setLightState(ui->widget_41, "light_getFromShelvesFinished", LightState::LightGreen) :
            setLightState(ui->widget_41, "light_getFromShelvesFinished", LightState::LightGray);
        data[ZWJF1 + shelvesId] == 1 ?
            setLightState(ui->widget_41, "light_putToShelvesFinished", LightState::LightGreen) :
            setLightState(ui->widget_41, "light_putToShelvesFinished", LightState::LightGray);
        data[WKXQ1 + fixtureId] == 1 ?
            setLightState(ui->widget_41, "light_getFromBoxFinished", LightState::LightGreen) :
            setLightState(ui->widget_41, "light_getFromBoxFinished", LightState::LightGray);
        data[WKXF1 + fixtureId] == 1 ?
            setLightState(ui->widget_41, "light_putToBoxFinished", LightState::LightGreen) :
            setLightState(ui->widget_41, "light_putToBoxFinished", LightState::LightGray);
    });
    connect(m_robotController.get(), &RobotController::sig_stateChanged, this, [=](int flag) {
        if (flag == 1) {    // 连接状态
            ui->btn_robotConnect->setText("Disconnect");
            setLightState(ui->widget_lightContainer, "light_robotConnect", LightState::LightGreen);
            // 连接成功后每隔 1s 读取一次机器人PLC数据
            if (m_comTimer && !m_comTimer->isActive()) {
                m_comTimer->setInterval(1000);
                m_comTimer->start();
            }
        } else if (flag == 0) {     // 未连接状态
            ui->btn_robotConnect->setText("Connect");
            setLightState(ui->widget_lightContainer, "light_robotConnect", LightState::LightGray);
            if (m_comTimer && m_comTimer->isActive()) {
                m_comTimer->stop();
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
    // 连接状态发生改变
    connect(m_ioController.get(), &IOController::sig_stateChanged, this, [=](int flag) {
        if (flag == 1) {    // 连接状态
            ui->btn_moduleConnect->setText("Disconnect");
            setLightState(ui->widget_lightContainer, "light_moduleState", LightState::LightGreen);
            // 连接成功后每隔 1000ms 读取一次IO模块数据
            if (m_ioComTimer && !m_ioComTimer->isActive()) {
                m_ioComTimer->setInterval(1000);
                m_ioComTimer->start();
            }
        } else if (flag == 0) {     // 未连接状态
            ui->btn_moduleConnect->setText("Connect");
            setLightState(ui->widget_lightContainer, "light_moduleState", LightState::LightGray);
            if (m_ioComTimer && m_ioComTimer->isActive()) {
                m_ioComTimer->stop();
            }
        }
    });
    // 通信发生错误
    connect(m_ioController.get(), &IOController::sig_errorOccurred, this, [=]() {
        setLightState(ui->widget_lightContainer, "light_moduleState", LightState::LightRed);
    });
    // 通信数据更新
    connect(m_ioController.get(), &IOController::sig_updateData, this, [=](const QVector<quint16>& data) {
        if (data.size() == IO_READ_NUMS) {
            int fixtureId = ui->cBox_fixture->currentIndex() + 1;
            if (fixtureId == 1) {
                data[FIXTURE_1_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGray);
                data[FIXTURE_1_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGray);
                if (data[FIXTURE_1_IN1] == 1 && data[FIXTURE_1_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_1_IN1] == 0 && data[FIXTURE_1_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 2) {
                data[FIXTURE_2_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGray);
                data[FIXTURE_2_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGray);
                if (data[FIXTURE_2_IN1] == 1 && data[FIXTURE_2_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_2_IN1] == 0 && data[FIXTURE_2_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 3) {
                data[FIXTURE_3_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGray);
                data[FIXTURE_3_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGray);
                if (data[FIXTURE_3_IN1] == 1 && data[FIXTURE_3_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_3_IN1] == 0 && data[FIXTURE_3_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 4) {
                data[FIXTURE_4_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGray);
                data[FIXTURE_4_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGray);
                if (data[FIXTURE_4_IN1] == 1 && data[FIXTURE_4_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_4_IN1] == 0 && data[FIXTURE_4_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 5) {
                data[FIXTURE_5_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGray);
                data[FIXTURE_5_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGray);
                if (data[FIXTURE_5_IN1] == 1 && data[FIXTURE_5_IN2] == 1) {
                    ui->label_fixtureState->setText(QStringLiteral("上升"));
                } else if (data[FIXTURE_5_IN1] == 0 && data[FIXTURE_5_IN2] == 0) {
                    ui->label_fixtureState->setText(QStringLiteral("下压"));
                } else {
                    ui->label_fixtureState->setText(QStringLiteral("-"));
                }
            } else if (fixtureId == 6) {
                data[FIXTURE_6_IN1] == 1 ? setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn1", LightState::LightGray);
                data[FIXTURE_6_IN2] == 1 ? setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_fixtureIn2", LightState::LightGray);
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
                data[DOOR_1_IN1] == 1 ? setLightState(ui->widget_53, "light_doorIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorIn1", LightState::LightGray);
                data[DOOR_1_IN2] == 1 ? setLightState(ui->widget_53, "light_doorIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorIn2", LightState::LightGray);
                if (data[DOOR_1_IN1] == 0 && data[DOOR_1_IN2] == 1) {
                    ui->label_doorState->setText(QStringLiteral("开门"));
                } else if (data[DOOR_1_IN1] == 1 && data[DOOR_1_IN2] == 0) {
                    ui->label_doorState->setText(QStringLiteral("关门"));
                } else {
                    ui->label_doorState->setText(QStringLiteral("-"));
                }
            } else if (doorId == 2) {
                data[DOOR_2_IN1] == 1 ? setLightState(ui->widget_53, "light_doorIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorIn1", LightState::LightGray);
                data[DOOR_2_IN2] == 1 ? setLightState(ui->widget_53, "light_doorIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorIn2", LightState::LightGray);
                if (data[DOOR_2_IN1] == 0 && data[DOOR_2_IN2] == 1) {
                    ui->label_doorState->setText(QStringLiteral("开门"));
                } else if (data[DOOR_2_IN1] == 1 && data[DOOR_2_IN2] == 0) {
                    ui->label_doorState->setText(QStringLiteral("关门"));
                } else {
                    ui->label_doorState->setText(QStringLiteral("-"));
                }
            } else if (doorId == 3) {
                data[DOOR_3_IN1] == 1 ? setLightState(ui->widget_53, "light_doorIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorIn1", LightState::LightGray);
                data[DOOR_3_IN2] == 1 ? setLightState(ui->widget_53, "light_doorIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorIn2", LightState::LightGray);
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
                data[DOOR_LOCK_1_IN1] == 1 ? setLightState(ui->widget_53, "light_doorLockIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorLockIn1", LightState::LightGray);
                data[DOOR_LOCK_1_IN2] == 1 ? setLightState(ui->widget_53, "light_doorLockIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorLockIn2", LightState::LightGray);
                if (data[DOOR_LOCK_1_IN1] == 1 && data[DOOR_LOCK_1_IN2] == 0) {
                    ui->label_lockState->setText(QStringLiteral("解锁"));
                } else if (data[DOOR_LOCK_1_IN1] == 0 && data[DOOR_LOCK_1_IN2] == 1) {
                    ui->label_lockState->setText(QStringLiteral("关锁"));
                } else {
                    ui->label_lockState->setText(QStringLiteral("-"));
                }
            } else if (doorLockId == 2) {
                data[DOOR_LOCK_2_IN1] == 1 ? setLightState(ui->widget_53, "light_doorLockIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorLockIn1", LightState::LightGray);
                data[DOOR_LOCK_2_IN2] == 1 ? setLightState(ui->widget_53, "light_doorLockIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorLockIn2", LightState::LightGray);
                if (data[DOOR_LOCK_2_IN1] == 1 && data[DOOR_LOCK_2_IN2] == 0) {
                    ui->label_lockState->setText(QStringLiteral("解锁"));
                } else if (data[DOOR_LOCK_2_IN1] == 0 && data[DOOR_LOCK_2_IN2] == 1) {
                    ui->label_lockState->setText(QStringLiteral("关锁"));
                } else {
                    ui->label_lockState->setText(QStringLiteral("-"));
                }
            } else if (doorLockId == 3) {
                data[DOOR_LOCK_3_IN1] == 1 ? setLightState(ui->widget_53, "light_doorLockIn1", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorLockIn1", LightState::LightGray);
                data[DOOR_LOCK_3_IN2] == 1 ? setLightState(ui->widget_53, "light_doorLockIn2", LightState::LightGreen) : setLightState(ui->widget_53, "light_doorLockIn2", LightState::LightGray);
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
                data[FIXTURE_1_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGray);
            } else if (fixtureId == 2) {
                data[FIXTURE_2_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGray);
            } else if (fixtureId == 3) {
                data[FIXTURE_3_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGray);
            } else if (fixtureId == 4) {
                data[FIXTURE_4_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGray);
            } else if (fixtureId == 5) {
                data[FIXTURE_5_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGray);
            } else if (fixtureId == 6) {
                data[FIXTURE_6_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_fixtureOut", LightState::LightGray);
            }

            int doorId = ui->cBox_door->currentIndex() + 1;
            if (doorId == 1) {
                data[DOOR_1_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_doorOut", LightState::LightGray);
            } else if (doorId == 2) {
                data[DOOR_2_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_doorOut", LightState::LightGray);
            } else if (doorId == 3) {
                data[DOOR_3_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_doorOut", LightState::LightGray);
            }

            int doorLockId = ui->cBox_doorLock->currentIndex() + 1;
            if (doorLockId == 1) {
                data[DOOR_LOCK_1_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorLockOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_doorLockOut", LightState::LightGray);
            } else if (doorLockId == 2) {
                data[DOOR_LOCK_2_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorLockOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_doorLockOut", LightState::LightGray);
            } else if (doorLockId == 3) {
                data[DOOR_LOCK_3_OUT] == 1 ? setClickedLightState(ui->widget_53, "btn_doorLockOut", LightState::LightGreen) : setClickedLightState(ui->widget_53, "btn_doorLockOut", LightState::LightGray);
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
    ui->label_src->setText(QString(u8"货架 %1").arg(shelvesId));
    ui->label_dst->setText(QString(u8"治具 %1").arg(fixtureId));

    long long count = 0;
    // // 0. 检查指定温控箱是否在工作，内部是否有托盘，门是否为锁紧状态，机械臂是否为原点状态
    ui->textBrowser->clear();
    ui->textBrowser->append(QString(u8"正在将托盘从 %1 号货架运至 %2 号检测位...").arg(shelvesId).arg(fixtureId));

    m_robotController->clearAllBits();
    m_robotController->runRobot();

    // 1. 温控箱开锁
    // 检查流程是否被暂停
    if (m_processState == ProcessState::PAUSED) {
        ui->textBrowser->append(QString(u8"暂停运行..."));
        while (m_processState == ProcessState::PAUSED) {
            QCoreApplication::processEvents();
        }
        ui->textBrowser->append(QString(u8"恢复运行"));
    }
    // 检查流程是否被终止
    if (m_processState == ProcessState::STOPPED) {
        ui->textBrowser->append(QString(u8"流程终止"));
        return;
    }
    m_ioController->boxUnlocking(boxId);
    // 启动一个计数器
    count = 0;

    // 2. 等待温控箱开锁完成信号，温控箱开门
    while (m_ioController->getBoxLockState(boxId) != LOCK_STATE::UNLOCKING) {
        count++;
        // 超时重试一次
        if (count >= 1000000) {
            m_ioController->boxUnlocking(boxId);
            count = 0;
        }
        // 处理事件，防止卡死
        QCoreApplication::processEvents();
        // 检查流程是否被暂停
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        // 检查流程是否被终止
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 已开锁").arg(boxId));
    m_ioController->boxOpen(boxId);
    count = 0;

    // 3. 等待温控箱开门完成信号，治具上升
    while (m_ioController->getBoxOpenState(boxId) != DOOR_STATE::OPENING) {
        count++;
        // 超时重试一次
        if (count >= 1000000) {
            m_ioController->boxOpen(boxId);
            count = 0;
        }
        // 处理事件，防止卡死
        QCoreApplication::processEvents();
        // 检查流程是否被暂停
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        // 检查流程是否被终止
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 门已打开").arg(boxId));
    m_ioController->fixtureSet(fixtureId, 0);
    count = 0;

    // 4. 等待治具上升完成信号，给机械臂下发叉取托盘命令
    while (m_ioController->getFixtureState(fixtureId) != FIXTURE_STATE::UP) {
        count++;
        // 超时重试一次
        if (count >= 1000000) {
            m_ioController->fixtureSet(fixtureId, 0);
            count = 0;
        }
        // 处理事件，防止卡死
        QCoreApplication::processEvents();
        // 检查流程是否被暂停
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        // 检查流程是否被终止
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(QString(u8"治具 %1 已抬起").arg(fixtureId));
    m_robotController->getFromShelves(shelvesId);

    // 5. 等待机械臂返回地轨移动请求，移动地轨至货架点位
    while (!m_robotController->requireTrackMove(SHELVES_POSITION)) {
        // 处理事件，防止卡死
        QCoreApplication::processEvents();
        // 检查流程是否被暂停
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        // 检查流程是否被终止
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(u8"地轨移动中...");
    m_trackController->moveToTarget(SHELVES_POSITION);

    // 6. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        // 处理事件，防止卡死
        QCoreApplication::processEvents();
        // 检查流程是否被终止
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    m_trackController->setMove(false);
    // 检查流程是否被暂停，由于地轨到位信号不会持续，因此不能在循环中检查
    if (m_processState == ProcessState::PAUSED) {
        ui->textBrowser->append(QString(u8"暂停运行..."));
        while (m_processState == ProcessState::PAUSED) {
            QCoreApplication::processEvents();
        }
        ui->textBrowser->append(QString(u8"恢复运行"));
    }
    // 检查流程是否被终止
    if (m_processState == ProcessState::STOPPED) {
        ui->textBrowser->append(QString(u8"流程终止"));
        return;
    }
    ui->textBrowser->append(u8"地轨已移动至货架点位");
    ui->textBrowser->append(QString(u8"正在从 %1 号货架处夹取托盘...").arg(shelvesId));
    m_robotController->trackMoveArrived(SHELVES_POSITION);

    // 7. 等待机械臂返回叉取完成信号，给机械臂下发放置命令
    while (!m_robotController->resultOfGetFromShelves(shelvesId)) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(u8"托盘夹取完成");
    m_robotController->putToFixture(fixtureId);

    // 8. 等待机械臂返回地轨移动请求，移动地轨至检测箱点位
    if (boxId == 1) {
        while (!m_robotController->requireTrackMove(BOX_1_POSITION)) {
            QCoreApplication::processEvents();
            if (m_processState == ProcessState::PAUSED) {
                ui->textBrowser->append(QString(u8"暂停运行..."));
                while (m_processState == ProcessState::PAUSED) {
                    QCoreApplication::processEvents();
                }
                ui->textBrowser->append(QString(u8"恢复运行"));
            }
            if (m_processState == ProcessState::STOPPED) {
                ui->textBrowser->append(QString(u8"流程终止"));
                return;
            }
        }
        m_trackController->moveToTarget(BOX_1_POSITION);
    } else if (boxId == 2) {
        while (!m_robotController->requireTrackMove(BOX_2_POSITION)) {
            QCoreApplication::processEvents();
            if (m_processState == ProcessState::PAUSED) {
                ui->textBrowser->append(QString(u8"暂停运行..."));
                while (m_processState == ProcessState::PAUSED) {
                    QCoreApplication::processEvents();
                }
                ui->textBrowser->append(QString(u8"恢复运行"));
            }
            if (m_processState == ProcessState::STOPPED) {
                ui->textBrowser->append(QString(u8"流程终止"));
                return;
            }
        }
        m_trackController->moveToTarget(BOX_2_POSITION);
    } else if (boxId == 3) {
        while (!m_robotController->requireTrackMove(BOX_3_POSITION)) {
            QCoreApplication::processEvents();
            if (m_processState == ProcessState::PAUSED) {
                ui->textBrowser->append(QString(u8"暂停运行..."));
                while (m_processState == ProcessState::PAUSED) {
                    QCoreApplication::processEvents();
                }
                ui->textBrowser->append(QString(u8"恢复运行"));
            }
            if (m_processState == ProcessState::STOPPED) {
                ui->textBrowser->append(QString(u8"流程终止"));
                return;
            }
        }
        m_trackController->moveToTarget(BOX_3_POSITION);
    }
    ui->textBrowser->append(u8"地轨移动中...");

    // 9. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    m_trackController->setMove(false);
    // 检查流程是否被暂停，由于地轨到位信号不会持续，因此不能在循环中检查
    if (m_processState == ProcessState::PAUSED) {
        ui->textBrowser->append(QString(u8"暂停运行..."));
        while (m_processState == ProcessState::PAUSED) {
            QCoreApplication::processEvents();
        }
        ui->textBrowser->append(QString(u8"恢复运行"));
    }
    // 检查流程是否被终止
    if (m_processState == ProcessState::STOPPED) {
        ui->textBrowser->append(QString(u8"流程终止"));
        return;
    }
    if (boxId == 1) {
        m_robotController->trackMoveArrived(BOX_1_POSITION);
    } else if (boxId == 2) {
        m_robotController->trackMoveArrived(BOX_2_POSITION);
    } else if (boxId == 3) {
        m_robotController->trackMoveArrived(BOX_3_POSITION);
    }
    ui->textBrowser->append(QString(u8"地轨已移动至 %1 号工位").arg(boxId));
    ui->textBrowser->append(QString(u8"正在向 %1 号治具处放置托盘...").arg(fixtureId));

    // 10. 等待机械臂返回放置完成信号，机械臂信号清零，治具下压
    while (!m_robotController->resultOfPutToFixture(fixtureId)) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(QString(u8"治具 %1 已下压").arg(fixtureId));
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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 门已关闭").arg(boxId));
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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 已上锁").arg(boxId));
    m_trackController->moveToTarget(ORIGIN_POSITION);
    ui->textBrowser->append(u8"地轨回原点中...");
    count = 0;

    // 15. 等待地轨回原点完成，开始检测
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            return;
        }
    }
    m_trackController->setMove(false);
    ui->textBrowser->append(u8"放置完成！");

    while (true) {
        QCoreApplication::processEvents();
        count++;
        if (count >= 1000) {
            count = 0;
            break;
        }
    }
}

void MainWindow::unloading(int shelvesId, int boxId, int fixtureId)
{
    ui->label_src->setText(QString(u8"治具 %1").arg(fixtureId));
    ui->label_dst->setText(QString(u8"货架 %1").arg(shelvesId));

    // // 0. 检查指定温控箱是否在工作，内部是否有托盘，门是否为锁紧状态，机械臂是否为原点状态

    long long count = 0;

    ui->textBrowser->clear();
    ui->textBrowser->append(QString(u8"正在将托盘从 %1 号治具运至 %2 号置物架...").arg(fixtureId).arg(shelvesId));

    m_robotController->clearAllBits();
    m_robotController->runRobot();

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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString(u8"治具 %1 已抬起").arg(fixtureId));
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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 已解锁").arg(boxId));
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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 门已打开").arg(boxId));
    m_robotController->getFromFixture(fixtureId);

    // 5. 等待机械臂返回地轨移动请求，移动地轨至检测箱工位
    if (boxId == 1) {
        while (!m_robotController->requireTrackMove(BOX_1_POSITION)) {
            QCoreApplication::processEvents();
            if (m_processState == ProcessState::PAUSED) {
                ui->textBrowser->append(QString(u8"暂停运行..."));
                while (m_processState == ProcessState::PAUSED) {
                    QCoreApplication::processEvents();
                }
                ui->textBrowser->append(QString(u8"恢复运行"));
            }
            if (m_processState == ProcessState::STOPPED) {
                ui->textBrowser->append(QString(u8"流程终止"));
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_1_POSITION);
    } else if (boxId == 2) {
        while (!m_robotController->requireTrackMove(BOX_2_POSITION)) {
            QCoreApplication::processEvents();
            if (m_processState == ProcessState::PAUSED) {
                ui->textBrowser->append(QString(u8"暂停运行..."));
                while (m_processState == ProcessState::PAUSED) {
                    QCoreApplication::processEvents();
                }
                ui->textBrowser->append(QString(u8"恢复运行"));
            }
            if (m_processState == ProcessState::STOPPED) {
                ui->textBrowser->append(QString(u8"流程终止"));
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_2_POSITION);
    } else if (boxId == 3) {
        while (!m_robotController->requireTrackMove(BOX_3_POSITION)) {
            QCoreApplication::processEvents();
            if (m_processState == ProcessState::PAUSED) {
                ui->textBrowser->append(QString(u8"暂停运行..."));
                while (m_processState == ProcessState::PAUSED) {
                    QCoreApplication::processEvents();
                }
                ui->textBrowser->append(QString(u8"恢复运行"));
            }
            if (m_processState == ProcessState::STOPPED) {
                ui->textBrowser->append(QString(u8"流程终止"));
                qDebug() << "process stop";
                return;
            }
        }
        m_trackController->moveToTarget(BOX_3_POSITION);
    }
    ui->textBrowser->append(u8"地轨移动中...");

    // 6. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    if (m_processState == ProcessState::PAUSED) {
        ui->textBrowser->append(QString(u8"暂停运行..."));
        while (m_processState == ProcessState::PAUSED) {
            QCoreApplication::processEvents();
        }
        ui->textBrowser->append(QString(u8"恢复运行"));
    }
    if (m_processState == ProcessState::STOPPED) {
        ui->textBrowser->append(QString(u8"流程终止"));
        qDebug() << "process stop";
        return;
    }
    if (boxId == 1) {
        m_robotController->trackMoveArrived(BOX_1_POSITION);
    } else if (boxId == 2) {
        m_robotController->trackMoveArrived(BOX_2_POSITION);
    } else if (boxId == 3) {
        m_robotController->trackMoveArrived(BOX_3_POSITION);
    }
    ui->textBrowser->append(QString(u8"地轨已移动至 %1 号工位").arg(boxId));
    ui->textBrowser->append(QString(u8"正在从 %1 号治具处叉取托盘...").arg(fixtureId));

    // 7. 等待机械臂返回叉取完成信号，给机械臂下发放置命令
    while (!m_robotController->resultOfGetFromFixture(fixtureId)) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(u8"托盘夹取完成");
    m_robotController->putToShelves(shelvesId);

    // 8. 等待机械臂返回地轨移动请求，移动地轨至货架位
    while (!m_robotController->requireTrackMove(SHELVES_POSITION)) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(u8"地轨移动中...");
    m_trackController->moveToTarget(SHELVES_POSITION);

    // 9. 等待地轨移动完成信号，给机械臂下发移动到位信号
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    if (m_processState == ProcessState::PAUSED) {
        ui->textBrowser->append(QString(u8"暂停运行..."));
        while (m_processState == ProcessState::PAUSED) {
            QCoreApplication::processEvents();
        }
        ui->textBrowser->append(QString(u8"恢复运行"));
    }
    if (m_processState == ProcessState::STOPPED) {
        ui->textBrowser->append(QString(u8"流程终止"));
        qDebug() << "process stop";
        return;
    }
    m_robotController->trackMoveArrived(SHELVES_POSITION);
    ui->textBrowser->append(u8"地轨已移动至货架点位");
    ui->textBrowser->append(QString(u8"正在向 %1 号货架处放置托盘...").arg(shelvesId));

    // 10. 等待机械臂返回放置完成信号，机械臂信号清零，温控箱关门
    while (!m_robotController->resultOfPutToShelves(shelvesId)) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 门已关闭").arg(boxId));
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
        if (m_processState == ProcessState::PAUSED) {
            ui->textBrowser->append(QString(u8"暂停运行..."));
            while (m_processState == ProcessState::PAUSED) {
                QCoreApplication::processEvents();
            }
            ui->textBrowser->append(QString(u8"恢复运行"));
        }
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    ui->textBrowser->append(QString(u8"检测箱 %1 已上锁").arg(boxId));
    m_trackController->moveToTarget(ORIGIN_POSITION);
    ui->textBrowser->append(u8"地轨回原点中...");

    // 13. 等待地轨回原点完成，结束
    while (!m_trackController->getArrivedSignal()) {
        QCoreApplication::processEvents();
        if (m_processState == ProcessState::STOPPED) {
            ui->textBrowser->append(QString(u8"流程终止"));
            qDebug() << "process stop";
            return;
        }
    }
    m_trackController->setMove(false);
    ui->textBrowser->append(u8"放置完成！");
}

int MainWindow::startDetection(std::string devid, std::string subdevid, std::string chlid, std::string filename) {
    int ret = -1;

    //定义长度变量
    int send_len = 0;
    int recv_len = 0;
    //获取当前时间
    SYSTEMTIME systm;
    GetLocalTime(&systm);
    std::string code = std::to_string(systm.wYear) + std::to_string(systm.wMonth) + std::to_string(systm.wDay) + std::to_string(systm.wHour) + std::to_string(systm.wMinute) + std::to_string(systm.wSecond) + std::to_string(rand() % (1 + 100));

    //定义发送缓冲区和接受缓冲区
    std::string send_buf = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?> \n"
                      "<bts version=\"1.0\"> \n"
                      " <cmd>start</cmd> \n"
                      " <list count = \"1\"> \n"
                      "  <start ip=\"127.0.0.1\" devtype=\"27\" devid=\"" + devid + "\" subdevid=\"" + subdevid + "\" chlid=\"" + chlid + "\" barcode=\"" + code + "\">D:\\Program Files\\NEWARE\\BTSClient80\\StepManager\\Template\\" + filename + "</start> \n"
                                                                                                                                                         " </list> \n"
                                                                                                                                                         "</bts>\n\n#\r\n";
    char recv_buf[10240];
    //定义服务端套接字，接受请求套接字
    SOCKET s_server;
    //服务端地址客户端地址
    SOCKADDR_IN server_addr;
    initialization();
    //填充服务端信息
    server_addr.sin_family = AF_INET;
    //ip地址和端口号
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.250");
    server_addr.sin_port = htons(502);
    //创建套接字
    s_server = socket(AF_INET, SOCK_STREAM, 0);
    if (::connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        std::cout << "服务器连接失败！" << std::endl;
        WSACleanup();
    }

    //发送,接收数据
    send_len = send(s_server, send_buf.c_str(), 10240, 0);
    if (send_len < 0) {
        std::cout << "发送失败！" << std::endl;
    }
    recv_len = recv(s_server, recv_buf, 10240, 0);
    if (recv_len < 0) {
        std::cout << "接受失败！" << std::endl;
    }
    else {
        // std::cout << "服务端信息:" << recv_buf << std::endl;
        // 接收消息处理
        // 解析xml数据
        // 定义一个TiXmlDocument类指针
        std::string recv_str = recv_buf;
        std::replace(recv_str.begin(), recv_str.end(), '#', ' ');
        TiXmlDocument* tinyXmlDoc = new TiXmlDocument();
        tinyXmlDoc->Parse(recv_str.c_str());
        TiXmlElement* bts = new TiXmlElement("bts");
        bts = tinyXmlDoc->FirstChildElement();
        TiXmlElement* cmd = new TiXmlElement("cmd");
        cmd = bts->FirstChildElement();
        TiXmlElement* list = new TiXmlElement("list");
        list = cmd->NextSiblingElement();
        for (TiXmlElement* elem = list->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement()) {
            if (std::string(elem->GetText()) == "ok") {
                std::cout << "设备号：" << elem->Attribute("devid") << " 单元号：" << elem->Attribute("subdevid") << " 通道号：" << elem->Attribute("chlid") << " 启动成功" << std::endl;
                ret = 1;
            }
            else {
                std::cout << "设备号：" << elem->Attribute("devid") << " 单元号：" << elem->Attribute("subdevid") << " 通道号：" << elem->Attribute("chlid") << " 启动失败" << std::endl;
            }
        }
    }

    //关闭套接字
    closesocket(s_server);
    //释放DLL资源
    WSACleanup();

    return ret;
}

std::vector<std::string> MainWindow::getDetectionStatus(const std::vector<std::vector<int>>& channelsList)
{
    //定义长度变量
    int send_len = 0;
    int recv_len = 0;

    //定义发送缓冲区和接受缓冲区
    std::string send_buf = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?> \n"
                      "<bts version=\"1.0\"> \n"
                      " <cmd>getchlstatus</cmd> \n"
                      " <list count = \"1\"> \n";

    for (int i = 0; i < channelsList.size(); ++i) {
        QString tmp = QString("  <status ip=\"127.0.0.1\" devtype=\"27\" devid=\"%1\" subdevid=\"%2\" chlid=\"%3\">true</status> \n").arg(channelsList[i][0]).arg(channelsList[i][1]).arg(channelsList[i][2]);
        send_buf += tmp.toStdString();
    }

    send_buf += " </list> \n"
                "</bts>\n\n#\r\n";



    char recv_buf[10240];
    //定义服务端套接字，接受请求套接字
    SOCKET s_server;
    //服务端地址客户端地址
    SOCKADDR_IN server_addr;
    initialization();
    //填充服务端信息
    server_addr.sin_family = AF_INET;
    //固定ip地址和端口号
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.250"); //运行ip
    server_addr.sin_port = htons(502);
    //创建套接字
    s_server = socket(AF_INET, SOCK_STREAM, 0);
    if (::connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        std::cout << "服务器连接失败！" << std::endl;
        WSACleanup();
    }

    //发送,接收数据
    send_len = send(s_server, send_buf.c_str(), 10240, 0);
    if (send_len < 0) {
        std::cout << "发送失败！" << std::endl;
    }
    recv_len = recv(s_server, recv_buf, 10240, 0);
    if (recv_len < 0) {
        std::cout << "接受失败！" << std::endl;
    }
    //关闭套接字
    closesocket(s_server);
    //释放DLL资源
    WSACleanup();

    // 接收消息处理
    // 解析xml数据
    if (recv_len > 0) {
        // 定义一个TiXmlDocument类指针
        std::string recv_str = recv_buf;
        replace(recv_str.begin(), recv_str.end(), '#', ' ');
        TiXmlDocument* tinyXmlDoc = new TiXmlDocument();
        tinyXmlDoc->Parse(recv_str.c_str());
        TiXmlElement* bts = new TiXmlElement("bts");
        bts = tinyXmlDoc->FirstChildElement();
        TiXmlElement* cmd = new TiXmlElement("cmd");
        cmd = bts->FirstChildElement();
        TiXmlElement* list = new TiXmlElement("list");
        list = cmd->NextSiblingElement();

        std::vector<std::string> status;
        for (TiXmlElement* elem = list->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement()) {
            status.push_back(std::string(elem->GetText()));
        }

        for (const auto& item : status) {
            std::cout << item << std::endl;
        }

        return status;

        // // 如果反馈为finish的通道数和stop的通道数之和等于查询的总通道数，则可以取出
        // if ((count(status.begin(), status.end(), "finish") + count(status.begin(), status.end(), "stop")) == status.size()) {
        //     //执行把电池取出操作
        //     std::cout << "取出电池..." << std::endl;
        // }
    }

    return {};
}


void MainWindow::initialization() {
    // 初始化套接字库
    WORD w_req = MAKEWORD(2, 2); // 版本号
    WSADATA wsadata;
    int err;
    err = WSAStartup(w_req, &wsadata);
    if (err != 0) {
        std::cout << "初始化套接字库失败" << std::endl;
    }
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
        std::cout << "套接字库版本不符" << std::endl;
        WSACleanup();
    }
    // 填充服务端地址信息
}

void MainWindow::loadChannelFile(int fixtureID)
{
    // ui->tableWidget_channelList->clear();
    // 读取ini文件
    QSettings settings("C:\\Users\\panrou\\Desktop\\RobotProject\\config.ini", QSettings::IniFormat);
    // 指定节
    settings.beginGroup(QString("Fixture%1").arg(fixtureID));
    // 获取该节下的所有键
    QStringList keys = settings.childKeys();
    // 遍历所有键
    foreach (const QString& key, keys) {
        QStringList res = settings.value(key).toString().split(":");
        if (res.size() != 3) {
            continue;
        }
        QString deviceID = res.at(0);
        QString cellID = res.at(1);
        QString channelID = res.at(2);
        ui->tableWidget_channelList->insertRow(ui->tableWidget_channelList->rowCount());
        QTableWidgetItem* item;
        item = new QTableWidgetItem(deviceID);
        ui->tableWidget_channelList->setItem(ui->tableWidget_channelList->rowCount() - 1, 0, item);
        item = new QTableWidgetItem(cellID);
        ui->tableWidget_channelList->setItem(ui->tableWidget_channelList->rowCount() - 1, 1, item);
        item = new QTableWidgetItem(channelID);
        ui->tableWidget_channelList->setItem(ui->tableWidget_channelList->rowCount() - 1, 2, item);
        QPushButton* btn = new QPushButton("-");
        btn->setStyleSheet("QPushButton {\
                                background-color: rgb(85, 85, 127);\
                                color: #fff;\
                            }\
                            \
                            QPushButton::pressed {\
                                background-color: rgb(69, 69, 103);\
                                color: #fff;\
                            }");
        ui->tableWidget_channelList->setCellWidget(ui->tableWidget_channelList->rowCount() - 1, 3, btn);
    }
    // 退出节
    settings.endGroup();
}


