#ifndef IOCONTROLLER_H
#define IOCONTROLLER_H

#include <QObject>

#include "ModbusCom.h"
#include "GlobalVariable.h"

enum class LOCK_STATE {
    OTHER,
    LOCKING,
    UNLOCKING
};

enum class DOOR_STATE {
    OTHER,
    OPENING,
    CLOSING
};

enum class FIXTURE_STATE {
    OTHER,
    UP,
    DOWN
};

class IOController : public QObject
{
    Q_OBJECT
public:
    IOController();

    void connectToPLC(const QString& ip, int port);
    void disconnectWithPLC();
    void reconnectToPLC();

    // 温控箱开门
    void boxOpen(int index);
    // 温控箱关门
    void boxClose(int index);
    // 温控箱锁紧
    void boxLocking(int index);
    // 温控箱解锁
    void boxUnlocking(int index);
    // 查询温控箱门的开关状态
    DOOR_STATE getBoxOpenState(int index);
    // 查询温控箱门的锁紧状态
    LOCK_STATE getBoxLockState(int index);
    // 设置治具气缸  1：下压   0：上抬
    int fixtureSet(int index, int value);
    // 获取治具气缸状态    1: 下压    0:上抬
    FIXTURE_STATE getFixtureState(int index);

    void requestFeedback(int flag);

    bool isConnected();

public slots:
    void resetAll();

private:
    void writeBit(int port, int value);
    int readBit(int port);

signals:
    void sig_stateChanged(int flag);
    void sig_updateData(const QVector<quint16>& data);
    void sig_errorOccurred();

private slots:
    void slot_stateChanged(bool flag);
    void slot_receivedData(const QVector<quint16>& data);

private:
    MyModbus* _modbus;
    QVector<quint16> _data;
    bool m_isConnected;
    QString m_lastIP;
    int m_lastPort;
};

#endif // IOCONTROLLER_H
