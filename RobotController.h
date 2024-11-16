#ifndef ROBOTCONTROLLER_H
#define ROBOTCONTROLLER_H

#include <QObject>

#include "ModbusCom.h"
#include "GlobalVariable.h"

class RobotController : public QObject
{
    Q_OBJECT
public:
    RobotController();

    void connectToPLC(const QString& ip, int port);
    void disconnectWithPLC();

    // 从货架上叉取托盘
    void getFromShelves(int index);
    bool resultOfGetFromShelves(int index);
    // 将托盘放置到治具
    void putToFixture(int index);
    bool resultOfPutToFixture(int index);
    // 从治具上叉取托盘
    void getFromFixture(int index);
    bool resultOfGetFromFixture(int index);
    // 将托盘放置到货架
    void putToShelves(int index);
    bool resultOfPutToShelves(int index);

    // 外部暂停
    void pauseRobot();
    void runRobot();
    void stopRobot();

    // 主程序调用
    void runProcess();
    // 外部伺服上电
    void applyElec();
    // 报警复位
    void resetWarning();

    bool requireTrackMove(int pos);
    void trackMoveArrived(int pos);
    void clearAllBits();

    void requestFeedback();
    void executeAction(int bit);
    void executeAction(int bit1, int bit2);

    bool isConnected();

signals:
    void sig_stateChanged(int flag);
    void sig_updateData(const QVector<quint16>& data);

private slots:
    void slot_stateChanged(bool flag);
    void slot_receivedData(QVector<quint16> data);

private:
    int getFeedback(int port);
    void analyseFeedback(const QVector<quint16>& data);

private:
    MyModbus* _modbus;
    QVector<quint16> m_data;
    bool m_isConnected;
};

#endif // ROBOTCONTROLLER_H
