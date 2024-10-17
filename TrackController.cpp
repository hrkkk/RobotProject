#include "TrackController.h"

#include <QVariant>
#include <QTimer>

TrackController::TrackController() : m_isConnected(false)
{
    _modbus = new MyModbus();
    connect(_modbus, &MyModbus::signal_stateChanged, this, &TrackController::slot_stateChanged);
    connect(_modbus, &MyModbus::signal_receivedData, this, &TrackController::slot_receivedData);
}

void TrackController::connectToPLC(const QString& ip, int port)
{
    _modbus->connectToModbus(ip, port);
}

void TrackController::disconnectWithPLC()
{
    _modbus->disconnect();
}

bool TrackController::isConnected()
{
    return m_isConnected;
}

void TrackController::reset(bool flag)
{
    if (flag) {
        if(!_modbus->writeModbusData(2, TRACK_RESET_SET, 1)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    } else {
        if(!_modbus->writeModbusData(2, TRACK_RESET_SET, 0)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    }
}

void TrackController::runToOrigin()
{
    setRunForward(false);
    setRunBackward(false);
    setRunToOrigin(true);
    setMove(true);
}

void TrackController::runForward()
{
    setRunBackward(false);
    setRunToOrigin(false);
    setRunForward(true);
    setMove(true);
}

void TrackController::runBackward()
{
    setRunToOrigin(false);
    setRunForward(false);
    setRunBackward(true);
    setMove(true);
}

void TrackController::stopMoving()
{
    setMove(false);
    setRunToOrigin(false);
    setRunForward(false);
    setRunBackward(false);
}

void TrackController::setRunToOrigin(bool flag)
{
    if (flag) {
        if(!_modbus->writeModbusData(2, TRACK_ORIGIN_SET, 1)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    } else {
        if(!_modbus->writeModbusData(2, TRACK_ORIGIN_SET, 0)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    }
}

void TrackController::setRunForward(bool flag)
{
    if (flag) {
        if(!_modbus->writeModbusData(2, TRACK_FORWARD_SET, 1)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    } else {
        if(!_modbus->writeModbusData(2, TRACK_FORWARD_SET, 0)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    }
}

void TrackController::setRunBackward(bool flag)
{
    if (flag) {
        if(!_modbus->writeModbusData(2, TRACK_BACKWARD_SET, 1)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    } else {
        if(!_modbus->writeModbusData(2, TRACK_BACKWARD_SET, 0)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    }
}

void TrackController::setPosition(int pos)
{
    if(!_modbus->writeModbusData(2, TRACK_POSITION_SET, pos)) {
        qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
    }
}

void TrackController::setMovingSpeed(int speed)
{
    if(!_modbus->writeModbusData(2, TRACK_VOLECITY_SET, speed)) {
        qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
    }
}

void TrackController::setMove(bool flag)
{
    if (flag) {
        if(!_modbus->writeModbusData(2, TRACK_MOVE_SET, 1)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    } else {
        if(!_modbus->writeModbusData(2, TRACK_MOVE_SET, 0)) {
            qDebug() << "Źż¾ŻŠ“ČėŹ§°Ü";
        }
    }
}

void TrackController::requestFeedback(int flag)
{
    if (flag == 1) {
        // ¶Į±£³Ö¼Ä“ęĘ÷
        // ÖøĮī£ŗ03
        // ĘšŹ¼µŲÖ·£ŗ2260
        // ¼Ä“ęĘ÷øöŹż£ŗ4
        if(!_modbus->readModbusData(3, TRACK_ORIGIN_GET, TRACK_READ_NUMS)) {
            qDebug() << "Źż¾Ż¶ĮČ”Ź§°Ü";
        }
    } else {
        // ¶Į±£³Ö¼Ä“ęĘ÷
        // ÖøĮī£ŗ03
        // ĘšŹ¼µŲÖ·£ŗ2202
        // ¼Ä“ęĘ÷øöŹż£ŗ13
        if(!_modbus->readModbusData(3, TRACK_POSITION_SET, TRACK_WRITE_NUMS)) {
            qDebug() << "Źż¾Ż¶ĮČ”Ź§°Ü";
        }
    }
}

void TrackController::slot_stateChanged(bool flag)
{
    m_isConnected = flag;
    emit sig_stateChanged(flag);
}

void TrackController::moveToTarget(int pos)
{
    // ¶ØĪ»Ī»ÖĆ
    setPosition(pos);

    QTimer::singleShot(100, [this]() {
        // ŃÓŹ±ŗóÖ“ŠŠµÄ“śĀė
        // Ęō¶Æ¶ØĪ»
        setMove(true);
    });
}


void TrackController::slot_receivedData(const QVector<quint16>& data)
{
    if (data.size() == TRACK_READ_NUMS) {
        m_data = data;
    }
    emit sig_updateData(data);
    // analyseFeedback(data);
}

void TrackController::analyseFeedback(const QVector<quint16>& data)
{
    // ½āĪöŹż¾Ż
    if (data[0] == 1 || data[1] == 1 || data[2] == 1) {    // »ŲŌ­µćĶź³É
        setMove(false);
        setRunBackward(false);
        setRunForward(false);
        setRunToOrigin(false);
        if (data[3] == 1) {
            emit sig_stateChanged(-1);
        } else {
            reset(false);
            emit sig_stateChanged(_modbus->getConnectionState());
        }
    }
}

bool TrackController::getArrivedSignal()
{
    return m_data[1];
}
